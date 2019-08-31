# Psaudo-quantum deblur

psaudo-quantum-deblur is a reverse function of Gaussian Blur.

a simulrated annealing.

```
$ qdeblur -i input-file -o output-file -t threshold
```

libgd graphics library is required.

## Principle

use UNIX /dev/urandom device. 

randomly change pixel -> Gaussian Blur -> calcurate PSNR with input.

PSNR for randomness control for fast single.

## Psaudo-quantum Reverse function

gdImageGaussianBlur is changed for other function by reason.

many montecalro AI are able to reverse function.

but "colorize" is not able. neural network is good for colorize.
