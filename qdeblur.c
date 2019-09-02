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
#include"ssim.h"

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

static long ssim_comp( void )
{
	#define SQ (KNL_SIZE*KNL_SIZE)
	double cand_pixels[COLORS * KNL_SIZE * KNL_SIZE ];
	double input_pixels [COLORS * KNL_SIZE * KNL_SIZE ];
	double ssim_all = 0;
	unsigned int x,y,c;
	long mse_all = 0;

	for( y=0 ; y < KNL_SIZE ; y++  )
	{
		for( x=0 ; x < KNL_SIZE ; x++  )
		{
			int color;
			color = gdImageGetTrueColorPixel( input_kernel , x , y );

			input_pixels[y * KNL_SIZE + x] = (double)gdTrueColorGetRed(color);
			input_pixels[SQ + y * KNL_SIZE + x] = (double)gdTrueColorGetGreen(color);
			input_pixels[SQ * 2 + y * KNL_SIZE + x] = (double)gdTrueColorGetBlue(color);
		}
	}

	cand_kernel = gdImageCopyGaussianBlurred( output_kernel , 4 , -1.0 );
/*	cand_kernel = gdImageCreateTrueColor( KNL_SIZE , KNL_SIZE );*/
/*	gdImageCopy(cand_kernel , output_kernel , 0,0, 0 , 0 , KNL_SIZE , KNL_SIZE ); */
/*	gdImageGaussianBlur(cand_kernel ); */ /* foward function */ 
	for( y=0 ; y < KNL_SIZE ; y++  )
	{
		for( x=0 ; x < KNL_SIZE ; x++  )
		{
			int color;
			color = gdImageGetTrueColorPixel( cand_kernel , x , y );

			cand_pixels[y * KNL_SIZE + x] = (double)gdTrueColorGetRed(color);
			cand_pixels[SQ + y * KNL_SIZE + x] = (double)gdTrueColorGetGreen(color);
			cand_pixels[SQ * 2 + y * KNL_SIZE + x] = (double)gdTrueColorGetBlue(color);
		}
	}
	ssim_all += SSIM( cand_pixels , input_pixels , KNL_SIZE , KNL_SIZE );
	ssim_all += SSIM( cand_pixels + SQ , input_pixels + SQ , KNL_SIZE , KNL_SIZE );
	ssim_all += SSIM( cand_pixels + SQ * 2 , input_pixels + SQ * 2 , KNL_SIZE , KNL_SIZE );

	gdImageDestroy( cand_kernel );
	return (long)(ssim_all / 3 * 255 );
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
	long ssim_all;

	for(counter = 0 ;;)
	{
		ssim_all = ssim_comp() ;
		if (  ssim_all / (  t ) == 0 )
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
					output_pixels[y][x][c] += (int)rand % ( ssim_all ) ;
					if(output_pixels[y][x][c] >= BRIGHT )
						output_pixels[y][x][c] = BRIGHT-1 ;
					else if(output_pixels[y][x][c] < 0 )
						output_pixels[y][x][c] = 0 ;
				}
			}
		}
		/*CPU cycle counter detects stall.*/
		if ( counter >= INT_MAX / (KNL_SIZE * KNL_SIZE * 7 ))
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
	for( y=0 ; y < input_SY - KNL_SIZE ; y += KNL_SIZE /2 )
	{
		for( x=0; x < input_SX - KNL_SIZE ; x += KNL_SIZE /2 )
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
