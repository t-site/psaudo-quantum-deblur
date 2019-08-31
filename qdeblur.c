/*
Copyright (c) 2019 "tropposite" as on https://github.com/t-site

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include<stdio.h>
#include<gd.h>
#include<string.h>
#include<limits.h>

#define KNL_SIZE 12
#define HALF_KNL_SZ 4
#define BLUR_KNL 5
#define MAX 255
#define BRIGHT 256
#define COLORS 3
#define RA_SIZE 4194304

static gdImagePtr input_kernel;
static gdImagePtr cand_kernel;
static gdImagePtr output_kernel;
static int mse[KNL_SIZE][KNL_SIZE][COLORS];
static unsigned int imageSX;
static unsigned int imageSY;
static FILE *randomfd;
static int t = 2;
static signed char randarray[RA_SIZE];
static int randpointer = 0;

static long psnr_mse( void )
{
	int cand_pixels[ KNL_SIZE ][ KNL_SIZE ][COLORS];
	int input_pixels [ KNL_SIZE ][ KNL_SIZE ][COLORS];
	unsigned int x,y,c;
	long mse_all = 0;

	for( y=0 ; y < KNL_SIZE ; y++  )
	{
		for( x=0 ; x < KNL_SIZE ; x++  )
		{
			int color;
			color = gdImageGetTrueColorPixel( input_kernel , x , y );

			input_pixels[y][x][0] = gdTrueColorGetRed(color);
			input_pixels[y][x][1] = gdTrueColorGetGreen(color);
			input_pixels[y][x][2] =  gdTrueColorGetBlue(color);
		}
	}
	cand_kernel = gdImageCreateTrueColor( KNL_SIZE , KNL_SIZE );
	gdImageCopy(cand_kernel , output_kernel , 0,0, 0 , 0 , KNL_SIZE , KNL_SIZE );
	gdImageGaussianBlur(cand_kernel );
	for( y=0 ; y < KNL_SIZE ; y++  )
	{
		for( x=0 ; x < KNL_SIZE ; x++  )
		{
			int color;
			color = gdImageGetTrueColorPixel( cand_kernel , x , y );

			cand_pixels[y][x][0] = gdTrueColorGetRed(color);
			cand_pixels[y][x][1] = gdTrueColorGetGreen(color);
			cand_pixels[y][x][2] =  gdTrueColorGetBlue(color);
		}
	}

	for( y = 0 ; y < KNL_SIZE ; y++)
	{
		for( x=0 ; x < KNL_SIZE ; x++ )
		{
			for ( c=0 ; c < COLORS ; c++ )
			{
				long a,b;
				a = (long)input_pixels[y][x][c];
				b = (long)cand_pixels[y][x][c];
				mse_all  += (( a - b )*( a - b ));
				mse[y][x][c] = ( a - b ) ;
				if( mse[y][x][c] < 0 )
					mse[y][x][c] = ( b - a )  ;
				if (mse[y][x][c] < 2 )
					mse[y][x][c] = 2;
			}
		}
	}
	gdImageDestroy( cand_kernel );
	return mse_all;
}
static signed char getrand(void)
{
	signed char rand;
	if( randpointer >= RA_SIZE )
	{
		randpointer = 0;
	}
	if(randpointer == 0)
	{
		fread(randarray,sizeof(signed char),RA_SIZE,randomfd);
	}
	rand = randarray[randpointer];
	randpointer++;
	return rand;

}

static int quantum_art( void )
{
	unsigned int x,y,c;
	int output_pixels [KNL_SIZE][KNL_SIZE][COLORS] ;
	int counter;
	long mse_all;

	for(counter = 0 ;;)
	{
		mse_all = psnr_mse() ;
		if (  mse_all / ( KNL_SIZE * KNL_SIZE * COLORS * MAX * t ) == 0 )
		{
			break;
		}
		for( y=0 ; y < KNL_SIZE ; y++ )
		{
			for( x=0 ; x < KNL_SIZE ; x++  )
			{
				int color;
				color = gdImageGetTrueColorPixel( output_kernel , x , y );
				output_pixels[y][x][0] = gdTrueColorGetRed(color);
				output_pixels[y][x][1] = gdTrueColorGetGreen(color);
				output_pixels[y][x][2] =  gdTrueColorGetBlue(color);
			}
		}
		for( y=0 ; y < KNL_SIZE ; y++)
		{
			for( x=0 ; x < KNL_SIZE ; x++)
			{
				for ( c=0 ; c<COLORS ; c++)
				{
	
					signed char rand;
					counter++;
					rand = getrand();
					output_pixels[y][x][c] += (int)rand % ( mse[y][x][c] /2 ) ;
					if(output_pixels[y][x][c] >= BRIGHT )
						output_pixels[y][x][c] = BRIGHT-1 ;
					else if(output_pixels[y][x][c] < 0 )
						output_pixels[y][x][c] = 0 ;
				}
			}
		}
		/*CPU cycle counter detects stall.*/
		if ( counter >= INT_MAX / (KNL_SIZE * 6 * 7 + KNL_SIZE * 2 * 7))
		{
			t++;
			printf ("t=%d\n",t);
			counter = 0;
		}
		for( y=0 ; y < KNL_SIZE ; y++ )
		{
			for( x=0 ; x < KNL_SIZE ; x++  )
			{
				int pixel;
				pixel = gdImageColorClosest(output_kernel,output_pixels[y][x][0],output_pixels[y][x][1],output_pixels[y][x][2]);
				gdImageSetPixel(output_kernel,x,y,pixel);
			}
		}
	}
	return 0;
}

static void twice(void)
{
	input_kernel = gdImageCreateTrueColor( KNL_SIZE , KNL_SIZE );
	output_kernel = gdImageCreateTrueColor( KNL_SIZE , KNL_SIZE );
}

gdImagePtr quantum_deblur(gdImagePtr input_image , int threshold)
{
	gdImagePtr output_image;
	unsigned int x,y;
	unsigned int input_SX,input_SY;
	t = threshold;
	input_SX = gdImageSX(input_image);
	input_SY = gdImageSY(input_image);
	randomfd = fopen("/dev/urandom","r");
	if(randomfd == NULL)
		return NULL;
	if ( input_SX < KNL_SIZE && input_SY < KNL_SIZE )
		return NULL;
	output_image = gdImageCreateTrueColor( input_SX , input_SY );
	gdImageCopy(output_image,input_image, 0 , 0 ,0 , 0 , input_SX , input_SY );
	if( output_image == NULL )
		return NULL;
	twice();
	for( y=0 ; y < input_SY - KNL_SIZE ; y++)
	{
		for( x=0; x < input_SX - KNL_SIZE ; x++)
		{
			gdImageCopy(input_kernel , input_image , 0,0, x , y , KNL_SIZE , KNL_SIZE );
			gdImageCopy(output_kernel , output_image , 0,0, x , y , KNL_SIZE , KNL_SIZE );
			quantum_art();
			/* TODO variable IN_KNL_SIZE , OUT_KNL_SIZE */
			gdImageCopy(output_image,output_kernel, x , y ,0 , 0 , KNL_SIZE , KNL_SIZE );
		}
	}
	fclose(randomfd);
	gdImageDestroy(input_kernel);	
	gdImageDestroy(output_kernel);
	return output_image;
}
