#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "./lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./lib/stb/stb_image_write.h"
#include <sys/time.h>

#define SIGMA 15

// http://pages.stat.wisc.edu/~mchung/teaching/MIA/reading/diffusion.gaussian.kernel.pdf.pdf
void generateGaussianKernel(double* k, int size, double sigma) {
    // double sigma = (size - 1) / 6.0;    
    // double sigma = SIGMA;
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

void applyFilter(unsigned char *in, unsigned char *out, int w, int h, int c, double* kernel, int kernel_size, int threads) {
    int kernel_pad = kernel_size / 2;
    size_t size = w * h;

    #pragma omp parallel for schedule(static, 64) num_threads(threads)
    for (long long int i = 0; i < size * c; i += c)
        if(i > kernel_pad * w * c && // Top
            i < (size * c - kernel_pad * w * c) && // Bottom
            i % (w * c) >= kernel_pad * c && // Left
            i % (w * c) < (w * c - kernel_pad * c)) // Right
            calculatePixel(in, out, i, w, h, c, kernel, kernel_size);
        else
            for (int j = 0; j < c; j++)
                out[i + j] = 0;
}

int main(int argc, char *argv[]) {
    if(argc < 4) {
        printf("Wrong arguments!\n");
        return -1;
    }

    struct timeval after, before, result;
    gettimeofday(&before, NULL);

    char *DIR_IMG_INPUT = argv[1];
    char *DIR_IMG_OUTPUT = argv[2];

    int KERNEL_SIZE = 3;
    sscanf(argv[3], "%d", &KERNEL_SIZE);

    int THREADS = atoi(argv[4]);

    double sigma = SIGMA;
    if(argv[5] != 0) sscanf(argv[5], "%lf", &sigma);

    int verbose;
    if(argv[6] != NULL) {
        sscanf(argv[6], "%d", &verbose);
        if(verbose != 1)
            verbose = 0;
    }    

    double kernel[KERNEL_SIZE][KERNEL_SIZE];
    generateGaussianKernel((double *) &kernel, KERNEL_SIZE, sigma);

    int width, height, channels;
    unsigned char *data = stbi_load(DIR_IMG_INPUT, &width, &height, &channels, STBI_default);

    if (data != NULL) {
        if(verbose) printf("Image dimensions: (%dpx, %dpx) and %d channels.\n", width, height, channels);
        
        unsigned char *output_image = malloc(width * height * channels);

        if(output_image == NULL) {
            printf("Error trying to allocate memory space");
            free(output_image);
            stbi_image_free(data);
            return -1;
        }
        
        applyFilter(data, output_image, width, height, channels, (double *) &kernel, KERNEL_SIZE, THREADS);

        // for (int i = 0; i < width * height * channels; i++) {
        //     printf("%f ", output_image[i]);
        //     if(i % width * height == 0)
        //         printf("\n");
        // }

        if (!stbi_write_png(DIR_IMG_OUTPUT, width, height, channels, output_image, width * channels))
            printf("Image cannot be created");

        free(output_image);
    } else {
        printf("Error loading the image");
    }

    gettimeofday(&after, NULL);
    timersub(&after, &before, &result);
    if(verbose) printf("\nTime elapsed: %ld.%06ld\n", (long int) result.tv_sec, (long int) result.tv_usec);
    else printf("%ld.%06ld\n", (long int)result.tv_sec, (long int)result.tv_usec);

    stbi_image_free(data);
}
