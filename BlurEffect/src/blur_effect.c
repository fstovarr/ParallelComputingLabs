#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"
#include <sys/time.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define SIGMA 15

struct Args {
    int id;
    int chunk_size;
    int threads;
    unsigned char *in;
    unsigned char *out;
    int w;
    int h;
    int c;
    double* kernel;
    int kernel_size;
};

// http://pages.stat.wisc.edu/~mchung/teaching/MIA/reading/diffusion.gaussian.kernel.pdf.pdf
void generateGaussianKernel(double* k, int size, double sigma) {
    double two_sigma_sq = 2.0 * sigma * sigma;

    double sum = 0.0;
    double res = 0.0;
    int mid_size = size / 2;

    for (int x = -mid_size; x <= mid_size; x++) 
        for (int y = -mid_size; y <= mid_size; y++) {
            int idx = (x + mid_size) * size + y + mid_size;
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

void calculatePixel(unsigned char *in, unsigned char *out, long long int i, int w, int h, int channels, double* kernel, int kernel_size) {
    int kernel_pad = kernel_size / 2;
    double v = 0.0, total = 0.0;

    for (int l = 0; l < channels; l++) {
        total = 0.0;
        for (int m = -kernel_pad; m <= kernel_pad; m++)
            for (int n = -kernel_pad; n <= kernel_pad; n++) {
                v = *(kernel + (m  + kernel_pad) * kernel_size + (n + kernel_pad));
                total += v * in[(i + l) +  (m * w * channels) + (n * channels)];
            }

        out[i + l] = total;
    }
}

void applyFilter(unsigned char *in, unsigned char *out, long long int start, long long int end, int w, int h, int c, double* kernel, int kernel_size) {
    int kernel_pad = kernel_size / 2;
    size_t size = w * h;

    for (long long int i = start * c; i < end * c; i += c)
        if(i > kernel_pad * w * c && // Top
            i < (size * c - kernel_pad * w * c) && // Bottom
            i % (w * c) >= kernel_pad * c && // Left
            i % (w * c) < (w * c - kernel_pad * c)) // Right
            calculatePixel(in, out, i, w, h, c, kernel, kernel_size);
        else
            for (int j = 0; j < c; j++)
                out[i + j] = 0;
}

void processImage(void *arg) {
    struct Args *args = arg;
    int id = args->id;
    int chunk_size = args->chunk_size;
    int threads = args->threads;
    int w = args->w;
    int h = args->h;
    int c = args->c;
    int kernel_size = args->kernel_size;
    double* kernel = args->kernel;
    unsigned char *in = args->in;
    unsigned char *out = args->out;

    size_t size = w * h;
    long long int end;

    for (long long int start = id * chunk_size; start < size; start += threads * chunk_size) {
        end = MIN(start + chunk_size, size);
        applyFilter(in, out, start, end, w, h, c, kernel, kernel_size);
    }
}

int main(int argc, char *argv[]) {
    if(argc < 5) {
        printf("Wrong arguments!\n");
        return -1;
    }

    struct timeval after, before, result;
    gettimeofday(&before, NULL);

    char *DIR_IMG_INPUT = argv[1];
    char *DIR_IMG_OUTPUT = argv[2];
    int KERNEL_SIZE = 3;
    sscanf(argv[3], "%d", &KERNEL_SIZE);
    if(KERNEL_SIZE % 2 == 0) {
        printf("The kernel size must be odd");
        return -1;
    }

    int THREADS = 4;
    sscanf(argv[4], "%d", &THREADS);

    int verbose;
    if(argv[6] == 0) verbose = 0;
    else sscanf(argv[6], "%d", &verbose);

    int sigma = SIGMA;
    if(argv[5] != 0) sscanf(argv[5], "%d", &sigma);

    double kernel[KERNEL_SIZE][KERNEL_SIZE];
    generateGaussianKernel((double *) &kernel, KERNEL_SIZE, sigma);

    unsigned char *data, *output_image;
    int width, height, channels;
    data = stbi_load(DIR_IMG_INPUT, &width, &height, &channels, STBI_default);

    if (data != NULL) {
        if(verbose) printf("\nImage dimensions: (%dpx, %dpx) and %d channels.\n", width, height, channels);
        
        output_image = malloc(width * height * channels);
        if(output_image == NULL) {
            printf("Error trying to allocate memory space");
            free(output_image);
            stbi_image_free(data);
            return -1;
        }
        
        pthread_t *threads = calloc(THREADS, sizeof(pthread_t));

        int chunk = 1;
        // int chunk = width * height / (THREADS * 10);
        // int chunk = width * height / (THREADS);

        struct Args *template = (struct Args *) calloc(THREADS, sizeof(struct Args));

        for (int i = 0; i < THREADS; i++) {
            (template + i)->id = i;
            (template + i)->chunk_size = chunk;
            (template + i)->threads = THREADS;
            (template + i)->w = width;
            (template + i)->h = height;
            (template + i)->c = channels;
            (template + i)->kernel_size = KERNEL_SIZE;
            (template + i)->kernel = (double *) &kernel;
            (template + i)->in = data;
            (template + i)->out = output_image;
            pthread_create(&threads[i], NULL, (void * (*)(void *)) processImage, (template + i));
        }

        for (int i = 0; i < THREADS; i++) 
            pthread_join(threads[i], NULL);

        if (!stbi_write_png(DIR_IMG_OUTPUT, width, height, channels, output_image, width * channels))
            printf("Image cannot be created");
        else
            if(verbose) printf("Image created");

        free(template);
        free(threads);
        free(output_image);
    } else {
        printf("Error loading the image");
    }

    gettimeofday(&after, NULL);
    timersub(&after, &before, &result);

    if(verbose) printf("\nTime elapsed: %ld.%06ld\n", (long int) result.tv_sec, (long int) result.tv_usec);
    else
        printf("%ld.%06ld\n", (long int)result.tv_sec, (long int)result.tv_usec);

    stbi_image_free(data);
}