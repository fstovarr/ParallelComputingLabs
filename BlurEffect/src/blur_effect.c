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

unsigned char *data, *output_image;

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
    int *he;
};

// http://pages.stat.wisc.edu/~mchung/teaching/MIA/reading/diffusion.gaussian.kernel.pdf.pdf
void generateGaussianKernel(double* k, int size) {
    // double sigma = (size - 1) / 6.0;    
    double sigma = 30;
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

void calculatePixel(int i, int w, int h, int channels, double* kernel, int kernel_size) {
    int kernel_pad = kernel_size / 2;
    double v = 0.0, total = 0.0;

    for (int l = 0; l < channels; l++) {
        total = 0.0;
        for (int m = -kernel_pad; m <= kernel_pad; m++)
            for (int n = -kernel_pad; n <= kernel_pad; n++) {
                v = *(kernel + (m  + kernel_pad) * kernel_size + (n + kernel_pad));
                // total += v * in[(i + l) + (m + kernel_pad) * channels + (n + kernel_pad) * channels];
                total += v * data[(i + l) +  (m * w * channels) + (n * channels )];
            }

        output_image[i + l] = total;
    }
}

void applyFilter(int w, int h, int c, double* kernel, int kernel_size, int start, int end) {
    int kernel_pad = kernel_size / 2;
    size_t size = w * h * c;

    for (int i = start; i < end; i += c)
        if(i > kernel_pad * w * c && // Top
            i < (size - kernel_pad * w * c) && // Bottom
            i % (w * c) >= kernel_pad * c && // Left
            i % (w * c) < (w * c - kernel_pad * c)) // Right
            calculatePixel(i, w, h, c, kernel, kernel_size);
        else
            for (int j = 0; j < c; j++)
                output_image[i + j] = 0;
}

// Chunk size (c*n)
void *processImage(void *arg) {
    struct Args *args = arg;
    int id = args->id;
    int chunk_size = args->chunk_size;
    int threads = args->threads;
    int w = args->w;
    int h = args->h;
    int c = args->c;
    int kernel_size = args->kernel_size;
    double* kernel = args->kernel;

    size_t size = w * h * c;
    int end;

    // printf("SIZE: %d ", size);

    int *b = args->he;

    // printf("Hilo %d | %d\n", id, b[id]);
    applyFilter(w, h, c, kernel, kernel_size, id * chunk_size * c, size/threads);

    // for (int start = id * chunk_size * c; start < size; start += threads * chunk_size * c) {
    //     end = MIN(start + threads * chunk_size * c, size);
    //     // memcpy(args->he + id + 1, args->he + id, sizeof(int));
    //     // printf("Hilo %d (%d, %d) \n", id, start, end);
    //     applyFilter(w, h, c, kernel, kernel_size, start, end);
    // }

    // printf("END %d", id);
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
    int KERNEL_SIZE = 3;
    sscanf(argv[3], "%d", &KERNEL_SIZE);
    if(KERNEL_SIZE % 2 == 0) {
        printf("The kernel size must be odd");
        return -1;
    }

    int THREADS = 4;
    sscanf(argv[4], "%d", &THREADS);

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
    data = stbi_load(DIR_IMG_INPUT, &width, &height, &channels, STBI_default);

    if (data != NULL) {
        printf("Image dimensions: (%dpx, %dpx) and %d channels.\n", width, height, channels);
        
        output_image = malloc(width * height * channels);
        if(output_image == NULL) {
            printf("Error trying to allocate memory space");
            free(output_image);
            stbi_image_free(data);
            return -1;
        }
        
        pthread_t *threads = calloc(THREADS, sizeof(pthread_t));
        int count[THREADS];

        for (int i = 0; i < THREADS; i++) {
            count[i] = 0;
            struct Args *template = (struct Args *)malloc(sizeof(struct Args));
            template->chunk_size = 6;
            template->threads = THREADS;
            template->w = width;
            template->h = height;
            template->c = channels;
            template->kernel_size = KERNEL_SIZE;
            // template->out = output_image;
            // template->in = data;
            template->kernel = kernel;
            template->id = i;
            template->he = count;
            pthread_create(&threads[i], NULL, &processImage, template);
        }

        for (int i = 0; i < THREADS; i++) 
            pthread_join(threads[i], NULL);

        // printf("\n");

        for (int i = 0; i < THREADS; i++)
            printf("%d ", count[i]);

        if (!stbi_write_png(DIR_IMG_OUTPUT, width, height, channels, output_image, width * channels))
            printf("Image cannot be created");
        else
            printf("Image created");

        // free(template);
        // free(threads);
        free(output_image);
    } else {
        printf("Error loading the image");
    }

    gettimeofday(&after, NULL);
    timersub(&after, &before, &result);
    printf("\nTime elapsed: %ld.%06ld\n", (long int) result.tv_sec, (long int) result.tv_usec);

    stbi_image_free(data);
}