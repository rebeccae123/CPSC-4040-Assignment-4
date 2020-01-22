Rebecca Edson
CPSC 4040 Homework 4
Convolution
October 20, 2019

Program can read image from file, display image, filter image
using convolution, and write image to file.

How to run code:
Compile with Makefile ("make").
Run with "./convolve <filter> <input image filename> <output image
  filename>" if you want to be able to write the image to a file,
  otherwise omit output image filename.
Press 'c' key to apply convolution to image. Can be applied as
  many times as desired. Filtered image is displayed.
Press 'r' key to display original image. Convolution resets.
Press 'w' key to write displayed image to output file from command
  line. Only do this if output file was specified. Supports ".png"
Press ESC to exit program/close window.

Assuming valid commandline arguments - no input validation.
Images and filters only supported if within same directory as program
  (no subdirectories).
