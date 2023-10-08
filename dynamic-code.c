#include <stdio.h>
#include <unistd.h>
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

    int *global_img = NULL;

    if(rank == 0) {
        global_img = malloc(WIDTH * HEIGHT * sizeof(int));
    }

    if(rank == 0) { // Master process

        int completed_rows = 0;
        MPI_Status status;
        for(int worker = 1; worker < size; worker++) {
            MPI_Send(&completed_rows, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);
            completed_rows++;
        }
        while(completed_rows < HEIGHT) {
            int row_data[WIDTH];
            MPI_Recv(row_data, WIDTH, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int worker = status.MPI_SOURCE;
            int row = status.MPI_TAG;
            for(int x = 0; x < WIDTH; x++) {
                global_img[row*WIDTH + x] = row_data[x];
            }
            MPI_Send(&completed_rows, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);
            completed_rows++;
        }
        for(int worker = 1; worker < size; worker++) {
            int dummy = -1;
            MPI_Send(&dummy, 1, MPI_INT, worker, 0, MPI_COMM_WORLD); // Send termination signal
        }
        
    } else { // Worker processes
        while(1) {

            int row;
            MPI_Status status;
            MPI_Recv(&row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,&status);
            if(row == -1) break; // Termination signal received
            int row_data[WIDTH];
            for(int x = 0; x < WIDTH; x++) {
                double real = (x - WIDTH / 2.0) * 4.0 / WIDTH;
                double imag = (row - HEIGHT / 2.0) * 4.0 / HEIGHT;
                row_data[x] = mandelbrot(real, imag);
            }
            MPI_Send(row_data, WIDTH,MPI_INT ,0 ,row ,MPI_COMM_WORLD);
        }
    }

    if(rank == 0) {

        gdImagePtr img = gdImageCreateTrueColor(WIDTH ,HEIGHT);
        for(int y=0;y<HEIGHT;y++){
           for(int x=0;x<WIDTH;x++){
               int value=global_img[y*WIDTH+x];
               int color=gdImageColorAllocate(img,value%256,value%256,value%256);
               gdImageSetPixel(img,x,y,color);
               
           }
       }
       FILE *pngout=fopen("mandelbrot-parallel.png","wb");
       gdImagePng(img,pngout);
       fclose(pngout);
       gdImageDestroy(img);

       free(global_img);
   }

   MPI_Finalize();

   end_timer=clock();
   cpu_time_used=((double)(end_timer-start_timer))/CLOCKS_PER_SEC;

   printf("Execution time: %f seconds\n",cpu_time_used);

   return 0;
}
