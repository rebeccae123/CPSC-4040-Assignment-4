/***********************
  Rebecca Edson
  CPSC 4040 Homework 4
  Convolution
  October 20, 2019
***********************/


#include <OpenImageIO/imageio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;
OIIO_NAMESPACE_USING


struct rgb_pixel {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

struct rgba_pixel {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
};

// Global variables
char *kernel;
string image_in, image_out;
int width, height, channels, n, pixformat;
double *weight;
unsigned char *original;
unsigned char *convolution;
bool display_original = 1;
bool already_convolved = 0;


/*
  Routine to read image file and store in pixmap
*/
void readImage() {

  ImageInput *in = ImageInput::open(image_in);  // open file
  if(!in) {
    cerr << "Could not open: " << image_in << ", error = ";
    cerr << geterror() << endl;
    return;
  }

  // get width and height of image
  const ImageSpec &spec = in->spec();
  width = spec.width;
  height = spec.height;
  channels = spec.nchannels;

  // pixmap to store image
  original = new unsigned char[width*height*channels];

  // read image file, map pixels
  if(!in->read_image(TypeDesc::UINT8, original)) {
    cerr << "Could not read pixels from: " << image_in << ", error = ";
    cerr << geterror() << endl;
    ImageInput::destroy(in);
    return;
  }

  // close file
  if(!in->close()) {
    cerr << "Error closing: " << image_in << ", error = ";
    cerr << geterror() << endl;
    ImageInput::destroy(in);
  }

  ImageInput::destroy(in);

  // pixel format
  if(channels == 1)
    pixformat = GL_LUMINANCE;
  else if(channels == 3)
    pixformat = GL_RGB;
  else
    pixformat = GL_RGBA;

}


/*
  Routine to read filter file and store weights into array
*/
void parseFilter() {

  ifstream file(kernel);  // read filter file

  if(file) {
    file >> n;  // size of kernel

    // array to store weights
    weight = new double[n*n];

    // weight values of kernel
    double w;
    for(int i = 0; i < n; i++) {
      for(int j = 0; j < n; j++) {
        file >> w;
        int k = i * n + j;
        weight[k] = w;  // store in array
      }
    }
  }

}


/*
  Routine to divide weights by scale factor
*/
void scale() {

  double sum_pos = 0;
  double sum_neg = 0;

  // sum positive and negative weights
  for(int i = 0; i < n; i++) {
    for(int j = 0; j < n; j++) {
      int k = i * n + j;
      if(weight[k] < 0)
        sum_neg += weight[k];
      else
        sum_pos += weight[k];
    }
  }

  // save maximum magnitude as scale factor
  double max_mag = max(sum_pos, sum_neg);

  // divide weights by scale factor
  for(int i = 0; i < n; i++) {
    for(int j = 0; j < n; j++) {
      int k = i * n + j;
      weight[k] /= max_mag;
    }
  }

}


/*
  Routine to flip kernel vertically and horizontally
*/
void flip() {

  int i, j, k, l;
  double *temp = new double[n*n];  // temporary array

  // read in weights backwards and store in temp
  for(i = 0, k = n-1; i < n; i++, k--) {  // bottom to top
    for(j = 0, l = n-1; j < n; j++, l--) {  // left to right
      int x = i * n + j;
      int y = k * n + l;
      temp[x] = weight[y];
    }
  }

  weight = temp;  // set weights to flipped kernel

}


/*
  Routine to filter image using convolution
*/
void convolve() {

  parseFilter();  // get weights
  scale();  // divide by scale factor
  flip();  // flip kernel

  // array to store alpha channel values (if applicable)
  unsigned char alpha_channel[width*height];

  // grayscale image
  if(channels == 1) {
    unsigned char *before;
    if(already_convolved)
      before = convolution;  // convolve image again
    else
      before = original;

    // array to store filtered image
    unsigned char *after = new unsigned char[width*height];

    // filter pixels within boundary
    for(int i = n/2; i < height-(n/2); i++) {
      for(int j = n/2; j < width-(n/2); j++) {
        unsigned char temp[n*n];
        double sum = 0;
        // multiply surrounding pixels with kernel
        for(int k = 0, a = i-(n/2); k < n; k++, a++) {
          for(int l = 0, b = j-(n/2); l < n; l++, b++) {
            int x = k * n + l;
            int y = a * width + b;
            temp[x] = weight[x] * before[y];
            sum += temp[x];
          }
        }
        int m = i * width + j;
        after[m] = sum;  // set pixel to sum of multiplied pixels
      }
    }

    // set boundary values to zero
    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        int k = i * width + j;
        if(i < n/2 || i >= height-(n/2))
          after[k] = 0;
        else {
          if(j < n/2 || j >= width-(n/2))
            after[k] = 0;
        }
      }
    }

    convolution = after;
  }
  else {
    rgb_pixel *before;
    //  RGBA image
    if(channels == 4) {
      before = new rgb_pixel[width*height];
      rgba_pixel *alpha;  // pixel with alpha value
      if(already_convolved)
        alpha = (rgba_pixel *) convolution;
      else
        alpha = (rgba_pixel *) original;
      for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
          int k = i * width + j;
          before[k].r = alpha[k].r;
          before[k].g = alpha[k].g;
          before[k].b = alpha[k].b;
          alpha_channel[k] = alpha[k].a;  // save alpha value
        }
      }
    }
    // RGB image
    else {
      if(already_convolved)
        before = (rgb_pixel *)convolution;  // convolve image again
      else
        before = (rgb_pixel *)original;
    }

    rgb_pixel *after = new rgb_pixel[width*height];

    // filter pixels within boundary
    for(int i = n/2; i < height-(n/2); i++) {
      for(int j = n/2; j < width-(n/2); j++) {
        rgb_pixel temp[n*n];
        double rsum = 0;
        double gsum = 0;
        double bsum = 0;
        // multiply surrounding pixels with kernel
        for(int k = 0, a = i-(n/2); k < n; k++, a++) {
          for(int l = 0, b = j-(n/2); l < n; l++, b++) {
            int x = k * n + l;
            int y = a * width + b;
            temp[x].r = weight[x] * before[y].r;
            temp[x].g = weight[x] * before[y].g;
            temp[x].b = weight[x] * before[y].b;
            rsum += temp[x].r;
            gsum += temp[x].g;
            bsum += temp[x].b;
          }
        }
        // set pixel to sum of multiplied pixels
        int m = i * width + j;
        after[m].r = rsum;
        after[m].g = gsum;
        after[m].b = bsum;
      }
    }

    // set boundary values to zero
    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        int k = i * width + j;
        if(i < n/2 || i >= height-(n/2)) {
          after[k].r = 0;
          after[k].g = 0;
          after[k].b = 0;
          if(channels == 4)
            alpha_channel[k] = 0;
        }
        else {
          if(j < n/2 || j >= width-(n/2)) {
            after[k].r = 0;
            after[k].g = 0;
            after[k].b = 0;
            if(channels == 4)
              alpha_channel[k] = 0;
          }
        }
      }
    }

    // store filtered pixel with alpha channel
    if(channels == 4) {
      rgba_pixel *alpha = new rgba_pixel[width*height];
      for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
          int k = i * width + j;
          alpha[k].r = after[k].r;
          alpha[k].g = after[k].g;
          alpha[k].b = after[k].b;
          alpha[k].a = alpha_channel[k];
        }
      }
      convolution = (unsigned char *) alpha;
    }
    else
      convolution = (unsigned char *) after;
  }

  already_convolved = 1;

}


/*
  Routine to write filtered image pixmap to file
*/
void writeImage() {

  unsigned char *pixmap;
  if(display_original)
    pixmap = original;  // write original image if user did not press 'c'
  else
    pixmap = convolution;  // display convolved image

  // create oiio file handler for image
  ImageOutput *out = ImageOutput::create(image_out);
  if(!out) {
    cerr << "Could not create: " << image_out << ", error = ";
    cerr << OpenImageIO::geterror() << endl;
    return;
  }

  // open file for writing image
  // file header will indicate RGB image with dimensions of original image
  ImageSpec spec(width, height, channels, TypeDesc::UINT8);
  if(!out->open(image_out, spec)) {
    cerr << "Could not open " << image_out << ", error = ";
    cerr << geterror() << endl;
    ImageOutput::destroy(out);
    return;
  }

  // write image to file
  if(!out->write_image(TypeDesc::UINT8, pixmap)) {
    cerr << "Could not close " << image_out << ", error = ";
    cerr << geterror() << endl;
    ImageOutput::destroy(out);
    return;
  }

  ImageOutput::destroy(out);

}


/*
  Displays image on pixmap
*/
void displayImage() {

  unsigned char *pixmap;
  if(display_original)
    pixmap = original;  // display original image if user pressed 'r'
  else
    pixmap = convolution;  // display convolved image

  glClearColor(0, 0, 0, 0);  // clear background
  glClear(GL_COLOR_BUFFER_BIT);  // clear window

  glPixelZoom(1, -1);  // flip image vertically
  glRasterPos2i(-1, 1);  // set drawing position to top left corner

  glDrawPixels(width, height, pixformat, GL_UNSIGNED_BYTE, pixmap);  // display pixmap

  glFlush();  // flush OpenGL pipeline to viewport

}


/*
  Keyboard callback routine:
  'c' - convolve
  'r' - reload
  'w' - write
  'esc' - quit
*/
void handleKey(unsigned char key, int x, int y) {

  switch(key) {
    case 'c':
    case 'C':
      convolve();  // apply convolution to currently displayed image
      display_original = 0;
      glutPostRedisplay();
      break;

    case 'r':
    case 'R':
      display_original = 1;  // display original image
      already_convolved = 0;
      glutPostRedisplay();
      break;

    case 'w':
    case 'W':
      writeImage();  // write convolved image to file
      break;

    case 27:  // ESC
      exit(0);  // exit program

    default:
      return;
  }

}


int main(int argc, char *argv[]) {

  kernel = argv[1];
  image_in = argv[2];

  if(argc == 4)
    image_out = argv[3];

  readImage();

  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
  glutInitWindowSize(width, height);
  glutCreateWindow("convolve");

  glutKeyboardFunc(handleKey);
  glutDisplayFunc(displayImage);

  glutMainLoop();

  return 0;

}
