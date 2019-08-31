rm qdeblur
gcc -g main.c qdeblur.c `pkg-config gdlib --libs --cflags` -fstack-protector-strong -Wall -o qdeblur
