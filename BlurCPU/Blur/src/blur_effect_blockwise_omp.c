 
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <omp.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

// http://pages.stat.wisc.edu/~mchung/teaching/MIA/reading/diffusion.gaussian.kernel.pdf.pdf
void generateGaussianKernel(double* k, int size) {
    double sigma = (size - 1.0) / 6.0;    
    // double sigma = 30;
    double two_sigma_sq = 2.0 * sigma * sigma;

    double sum = 0.0;
    double res = 0.0;
    int mid_size = size / 2;

    for (int x = -mid_size; x <= mid_size; x++) 
        for (int y = -mid_size; y <= mid_size; y++) {
            int idx = (x + mid_size) * size + y + mid_size;
            // double r = sqrt();
            res = (double)(exp(-(x * x + y * y) / two_sigma_sq) / (two_sigma_sq * M_PI));
            memcpy(k + idx, &res, sizeof(res));
            sum += *(k + idx);
        }

    for (int x = 0; x < size; x++)
        for (int y = 0; y < size; y++) {
            res = *(k + x * size + y) / sum;
            memcpy(k + x * size + y, &res, sizeof(res));
        }
}

void calculatePixel(unsigned char *in, unsigned char *out, int i, int w, int h, int channels, double* kernel, int kernel_size) {
    int kernel_pad = kernel_size / 2;
    double v = 0.0, total = 0.0;

    for (int l = 0; l < channels; l++) {
        total = 0.0;
        for (int m = -kernel_pad; m <= kernel_pad; m++)
            for (int n = -kernel_pad; n <= kernel_pad; n++) {
                v = *(kernel + (m  + kernel_pad) * kernel_size + (n + kernel_pad));
                total += v * in[(i + l) +  (m * w * channels) + (n * channels )];
            }
        out[i + l] = total;
    }
}

void applyFilter(unsigned char *in, unsigned char *out, int w, int h, int c, double* kernel, int kernel_size, int number_of_threads )
{
  int thread_id = omp_get_thread_num();

  int kernel_pad = kernel_size / 2;
  size_t size = w * h * c;

  int delta = w * h / number_of_threads;//iterations for each thread

  int iter = delta;
  if( thread_id == number_of_threads - 1 )
    iter = w * h - ( number_of_threads - 1 ) * delta;

  int beg = delta * thread_id * c;
  for( int it = 0, i = beg; it < iter; ++ it, i += c )
  {
      if(i >= kernel_pad * w * c && // Top
          i < (size - kernel_pad * w * c) && // Bottom
          i % (w * c) >= kernel_pad * c && // Left
          i % (w * c) < (w * c - kernel_pad * c)) // Right
          calculatePixel(in, out, i, w, h, c, kernel, kernel_size);
      else
          for (int j = 0; j < c; j++)
              out[i + j] = 0;
  }
}

int main(int argc, char *argv[]) {
    if(argc != 5) {
        printf("Wrong arguments!\n");
        return -1;
    }
    struct timeval after, before, result;
    gettimeofday(&before, NULL);

    char *DIR_IMG_INPUT = argv[1];
    char *DIR_IMG_OUTPUT = argv[2];
    int KERNEL_SIZE = atoi(argv[3]);
    int NUMBER_OF_THREADS = atoi(argv[4]);
    int verbose = 0;

    double kernel[KERNEL_SIZE*KERNEL_SIZE];
    generateGaussianKernel(kernel, KERNEL_SIZE);

    int width, height, channels;
    unsigned char *data = stbi_load(DIR_IMG_INPUT, &width, &height, &channels, STBI_default);

    if (data != NULL) {
       if(verbose) printf("Image dimensions: (%dpx, %dpx) and %d channels.\n", width, height, channels);
        
        unsigned char *output_image = malloc(width * height * channels);
        if(output_image == NULL) {
            if(verbose)printf("Error trying to allocate memory space");
            free(output_image);
            stbi_image_free(data);
            return -1;
        }

        omp_set_num_threads( NUMBER_OF_THREADS );
        #pragma omp parallel
        {
          applyFilter(data, output_image, width, height, channels, (double *) &kernel, KERNEL_SIZE, NUMBER_OF_THREADS);
        }

        if(!stbi_write_png(DIR_IMG_OUTPUT, width, height, channels, output_image, width * channels))
            if(verbose)printf("Image cannot be created");

        free(output_image);
    } else {
        if(verbose)printf("Error loading the image");
    }

    gettimeofday(&after, NULL);
    timersub(&after, &before, &result);
    printf("%ld.%06ld\n", (long int) result.tv_sec, (long int) result.tv_usec);

    stbi_image_free(data);
    pthread_exit( 0 );
    return 0;
}