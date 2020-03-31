#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

// #define DIR_IMG_INPUT "img/favicon.png"
char *DIR_IMG_INPUT = "img/test.png";
// #define DIR_IMG_INPUT "img/test2.jpg"
char *DIR_IMG_OUTPUT = "out/out_test.png";

void generateGaussianKernel(double* k, int size) {
    double sigma = 1.0;
    double two_sigma_sq = 2 * sigma * sigma;

    double sum = 0.0;
    double res = 0.0;
    int mid_size = size / 2;

    for (int x = -mid_size; x <= mid_size; x++) 
        for (int y = -mid_size; y <= mid_size; y++) {
            int idx = (x + mid_size) * size + y + mid_size;
            double r = sqrt(x * x + y * y);
            res = (double)((1 / (two_sigma_sq * M_PI)) * exp(-r * r / two_sigma_sq));
            memcpy(k + idx, &res, sizeof(res));
            sum += *(k + idx);
            // printf("(%f %d)", *(k + idx), idx);
        }
        // printf("\n");

    // printf("SUM %f \n", sum);

    for (int x = 0; x < size; x++)
        for (int y = 0; y < size; y++) {
            res = *(k + x * size + y) / sum;
            memcpy(k + x * size + y, &res, sizeof(res));
        }
}

void calculatePixel(unsigned char *in, unsigned char *out, int i, int w, int h, int channels, double* kernel, int kernel_size) {
    int kernel_pad = kernel_size / 2;

    for (int l = 0; l < channels; l++) {
        double total = 0.0;
        for (int m = -kernel_pad; m <= kernel_pad; m++)
            for (int n = -kernel_pad; n <= kernel_pad; n++) {
                double v = *(kernel + (m  + kernel_pad) * kernel_size + (n + kernel_pad));
                total += v * in[(i + l) + (m + kernel_pad) * channels + (n + kernel_pad) * channels];
            }

        out[i + l] = total;
    }
}

void applyFilter(unsigned char *in, unsigned char *out, int w, int h, int c, double* kernel, int kernel_size) {
    int kernel_pad = kernel_size / 2;
    size_t size = w * h * c;

    for (int i = 0; i < size; i += c)
        if(i > kernel_pad * w * c && // Top
            i < (size - kernel_pad * w * c) && // Bottom
            i % (w * c) >= kernel_pad * c && // Left
            i % (w * c) < (w * c - kernel_pad * c)) // Right
            calculatePixel(in, out, i, w, h, c, kernel, kernel_size);
        else
            for (int j = 0; j < c; j++)
                out[i + j] = 0;
}

int main(int argc, char *argv[]) {
    if(argc != 4) {
        printf("Wrong arguments!\n");
        return -1;
    }
    
    int KERNEL_SIZE = (*argv[3]) - '0';
    DIR_IMG_INPUT = argv[1];
    DIR_IMG_OUTPUT = argv[2];

    printf("%s %s %d\n", DIR_IMG_INPUT, DIR_IMG_OUTPUT, KERNEL_SIZE);

    double kernel[KERNEL_SIZE][KERNEL_SIZE];
    generateGaussianKernel(kernel, KERNEL_SIZE);

    // Print kernel

    // for (int i = 0; i < KERNEL_SIZE; i++) {
    //     for (int j = 0; j < KERNEL_SIZE; j++)
    //         printf("%f ", t[i][j]);
    //     printf("\n");
    // }

    // printf("\n");

    int width, height, channels;
    unsigned char *data = stbi_load(DIR_IMG_INPUT, &width, &height, &channels, STBI_default);

    if (data != NULL) {
        printf("Image dimensions: (%dpx, %dpx) and %d channels.\n", width, height, channels);
        
        unsigned char *output_image = malloc(width * height * channels);
        applyFilter(data, output_image, width, height, channels, kernel, KERNEL_SIZE);

        stbi_write_png(DIR_IMG_OUTPUT, width, height, channels, output_image, width * channels);
    } else {
        printf("Error loading the image");
    }

    stbi_image_free(data);
}