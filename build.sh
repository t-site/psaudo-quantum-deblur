rm qdeblur
gcc -g main.c qdeblur.c ssim.c -lm `pkg-config gdlib --libs --cflags` -fstack-protector-strong -Wall -o qdeblur
