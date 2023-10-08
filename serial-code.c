// Include the necessary header files.
#include <stdio.h>
#include <gd.h>
#include <time.h>

// Define the width, height, and maximum iteration count for the Mandelbrot set.
#define WIDTH 800
#define HEIGHT 800
#define MAX_ITER 1000

// Define a function to calculate the Mandelbrot set for a given real and imaginary number.
int mandelbrot(double real, double imag) {
  // Initialize the iteration counter and complex number variables.
  int n;
  double r = 0.0;
  double i = 0.0;

  // Start the timer
  clock_t start, end;
  double cpu_time_used;

  start = clock();

  // Iterate over the maximum number of iterations.
  for (n = 0; n < MAX_ITER; n++) {
    // Calculate the square of the real and imaginary parts of the complex number.
    double r2 = r * r;
    double i2 = i * i;

    // If the sum of the squares of the real and imaginary parts is greater than 4, then the number is not in the Mandelbrot set.
    if (r2 + i2 > 4.0) {
      return n;
    }

    // Calculate the next iteration of the complex number.
    i = 2 * r * i + imag;
    r = r2 - i2 + real;
  }

  // If the maximum number of iterations is reached, then the number is in the Mandelbrot set.
  return n;
}

// Define the main function.
int main() {
  // Create a new gdImagePtr object to represent the Mandelbrot image.
  gdImagePtr img = gdImageCreateTrueColor(WIDTH, HEIGHT);

  // Start the timer.
  clock_t start, end;
  double cpu_time_used;

  start = clock();

  // Iterate over the pixels in th image.
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      // Calculate the real and imaginary components of the complex number for the current pixel.
      double real = (x - WIDTH / 2.0) * 4.0 / WIDTH;
      double imag = (y - HEIGHT / 2.0) * 4.0 / HEIGHT;

      // Calculate the Mandelbrot value for the current pixel.
      int value = mandelbrot(real, imag);

      // Allocate a color for the current pixel based on its Mandelbrot value.
      int color = gdImageColorAllocate(img, value % 256, value % 256, value % 256);

      // Set the color of the current pixel.
      gdImageSetPixel(img, x, y, color);
    }
  }

  // Save the Mandelbrot image to a PNG file.
  FILE *pngout = fopen("mandelbrot.png", "wb");
  gdImagePng(img, pngout);
  fclose(pngout);

  // Destroy the gdImagePtr object.
  gdImageDestroy(img);

  // Stop the timer and print out how long did it take to finish execution.
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("Execution time: %f seconds\n", cpu_time_used);

  // Return 0 to indicate success.
  return 0;
}
