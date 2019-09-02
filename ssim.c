/*sample code from https://dftalk.jp/?p=18111*/
#include  <math.h>
 
int mirror( const int inx, const int min, const int max){
	int x = inx;
    while(x < min || x >= max){
        if(x < min) x = min + (min - x); 
        if(x >= max) x = max + (max - x - 1);
    }
 
    return x;
}
 
double SQR(double x){
    return x * x;
}
 
double MSE(double *data1, double *data2, int width, int height){
 
    double Sum = 0.0;
 
    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){
            Sum += SQR(data2[x + y * width] - data1[x + y * width]);
        }
    }
    Sum /= (width * height);
 
    return Sum;
}
 
double PSNR(double *data1, double *data2, int width, int height){
    return 10.0 * log10(255.0 * 255.0 / MSE(data1, data2, width, height));
}
 
double SSIM(double *data1, double *data2, int width, int height){
 
    double c1 = SQR(0.01 * 255.0);
    double c2 = SQR(0.03 * 255.0);
 
    int m = 2;
 
    double Sum = 0.0;
 
    int mx, my;
 
    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){
 
            double ave1 = 0.0, ave2 = 0.0; // 平均
            double var1 = 0.0, var2 = 0.0; // 分散
            double cov = 0.0; // 共分散
 
            for(int dy=-m;dy<=m;dy++){
                for(int dx=-m;dx<=m;dx++){
                    mx = mirror(x + dx, 0, width);
                    my = mirror(y + dy, 0, height);
 
                    ave1 += data1[mx + my * width];
                    ave2 += data2[mx + my * width];
                }
            }
            ave1 /= SQR(m * 2.0 + 1.0);
            ave2 /= SQR(m * 2.0 + 1.0);
 
            for(int dy=-m;dy<=m;dy++){
                for(int dx=-m;dx<=m;dx++){
                    mx = mirror(x + dx, 0, width);
                    my = mirror(y + dy, 0, height);
 
                    var1 += SQR(data1[mx + my * width] - ave1);
                    var2 += SQR(data2[mx + my * width] - ave2);
                    cov += fabs(data1[mx + my * width] - ave1) * fabs(data2[mx + my * width] - ave2);
                }
            }
            var1 /= SQR(m * 2.0 + 1.0);
            var2 /= SQR(m * 2.0 + 1.0);
            cov /= SQR(m * 2.0 + 1.0);
 
            Sum += ((2.0 * ave1 * ave2 + c1) * (2.0 * cov + c2)) / ((SQR(ave1) + SQR(ave2) + c1) * (var1 + var2 + c2));
        }
    }
 
    return Sum / (width * height);
}
