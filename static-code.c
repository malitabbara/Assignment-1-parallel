#include <stdio.h>
#include <gd.h>
#include <mpi.h>
#include <time.h>

#define WIDTH 800
#define HEIGHT 800
#define MAX_ITER 1000

int mandelbrot(double real, double imag) {
  int n;
  double r = 0.0;
  double i = 0.0;
  for(n = 0; n < MAX_ITER; n++) {
    double r2 = r*r;
    double i2 = i*i;
    if(r2 + i2 > 4.0) {
      return n;
    }
    i = 2*r*i + imag;
    r = r2 - i2 + real;
  }
  return n;
}

int main(int argc, char *argv[]) {
  int rank, size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  clock_t start_timer, end_timer;
  double cpu_time_used;

  start_timer = clock();

  int *local_img = malloc(WIDTH * HEIGHT / size * sizeof(int));
  int *global_img = NULL;

  if(rank == 0) {
    global_img = malloc(WIDTH * HEIGHT * sizeof(int));
  }

  int start = rank * HEIGHT / size;
  int end = (rank+1) * HEIGHT / size;

  for(int y = start; y < end; y++) {
    for(int x = 0; x < WIDTH; x++) {
      double real = (x - WIDTH / 2.0) * 4.0 / WIDTH;
      double imag = (y - HEIGHT / 2.0) * 4.0 / HEIGHT;
      local_img[(y-start)*WIDTH + x] = mandelbrot(real, imag);
    }
  }

  // Gather the local images from all processes
  MPI_Gather(local_img, WIDTH*HEIGHT/size, MPI_INT, global_img, WIDTH*HEIGHT/size, MPI_INT, 0, MPI_COMM_WORLD);

  if(rank == 0) {
    gdImagePtr img = gdImageCreateTrueColor(WIDTH, HEIGHT);
    for(int y = 0; y < HEIGHT; y++) {
      for(int x = 0; x < WIDTH; x++) {
        int value = global_img[y*WIDTH + x];
        int color = gdImageColorAllocate(img, value % 256, value % 256, value % 256);
        gdImageSetPixel(img, x, y, color);
      }
    }
    FILE *pngout = fopen("mandelbrot-parallel.png", "wb");
    gdImagePng(img, pngout);
    fclose(pngout);
    gdImageDestroy(img);
  }

  free(local_img);
  if(rank == 0) {
    free(global_img);
  }

  MPI_Finalize();

  end_timer = clock();
  cpu_time_used = ((double) (end_timer - start_timer)) / CLOCKS_PER_SEC;

  printf("Execution time: %f seconds\n", cpu_time_used);

  return 0;
}
