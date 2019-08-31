rm qdeblur
gcc -O2 -msse4.1 main.c qdeblur.c `pkg-config gdlib --libs --cflags` -fstack-protector-strong -Wall -o qdeblur
