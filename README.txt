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
Experiments completed and successful with:
squares.png - pulse.filt, tent.filt, hp.filt, sobol-horiz.filt,
  sobol-vert.filt
checkers.png - box.filt, box5.filt, box7.filt, box9.filt
biglines.png - cross.filt
square.png - diagonal.filt
waves.png - sobol-horiz.filt, sobol-vert.filt, define1.filt, define2.filt
