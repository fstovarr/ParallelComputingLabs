// !./bin/blur-effect $INPUT_FILE $OUTPUT_FILE $KERNEL_SIZE $SIGMA $VERBOSE

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"
#include <sys/time.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>

#define SIGMA 15
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define CHECK(call)                                                            \
{                                                                              \
    const cudaError_t error = call;                                            \
    if (error != cudaSuccess)                                                  \
    {                                                                          \
        fprintf(stderr, "Error: %s:%d, ", __FILE__, __LINE__);                 \
        fprintf(stderr, "code: %d, reason: %s\n", error,                       \
                cudaGetErrorString(error));                                    \
    }                                                                          \
}

#if !defined(__CUDA_ARCH__) || __CUDA_ARCH__ >= 600
#else
__device__ double atomicAdd(double* a, double b) { return b; }
#endif

__global__ void calculatePi(double *piTotal, int totalThreads)
{   
    long int i = 0;
    int index = (blockDim.x * blockIdx.x) + threadIdx.x;
     
    __syncthreads();
    if(index == 0){
        for(i = 1; i < totalThreads; i++)
            piTotal[0] += 1;
    }
}

// http://pages.stat.wisc.edu/~mchung/teaching/MIA/reading/diffusion.gaussian.kernel.pdf.pdf
__global__ void generateGaussianKernel(double *k, double *accumulation, int size, double sigma) {
    int idx = blockDim.x * blockIdx.x + threadIdx.x;
    int x = idx % size, y = idx / size;
    int i = 0;

    if(idx < size * size) {
        k[idx] = (double)(exp(-(x * x + y * y) / (2.0 * sigma * sigma)) / (2.0 * sigma * sigma * M_PI));
        atomicAdd(accumulation, k[idx]);
    }

    __syncthreads();

    if(idx == 0) {
      for (i = 0; i < (size * size); i++)
        k[i] /= *accumulation;
    }    
}

__device__ void calculatePixel(unsigned char *in, unsigned char *out, long int i, int w, int h, int channels, double* kernel, int kernel_size) {
    int kernel_pad = kernel_size / 2, idx;
    double v = 0.0, total = 0.0;

    for (int l = 0; l < channels; l++) {
        total = 0.0;
        for (int m = -kernel_pad; m <= kernel_pad; m++)
            for (int n = -kernel_pad; n <= kernel_pad; n++) {
                v = kernel[(m  + kernel_pad) * kernel_size + (n + kernel_pad)];
                idx = ((i + l) + (m * w * channels) + (n * channels)) % (w * h * channels);
                total += v * (in[idx]);
            }
        out[i + l] = total;
    }
}

__global__ void applyFilter(unsigned char *in, unsigned char *out, double *kernel, int w, int h, int c, int kernel_size) {
    int kernel_pad = kernel_size / 2;
    size_t size = w * h * c;
    long int idx = (blockDim.x * blockIdx.x + threadIdx.x) * c;

    if(idx < size)
        if(idx >= kernel_pad * w * c && // Top
            idx < (size - kernel_pad * w * c) && // Bottom
            idx % (w * c) >= kernel_pad * c && // Left
            idx % (w * c) < (w * c - kernel_pad * c)) // Right
            calculatePixel(in, out, idx, w, h, c, kernel, kernel_size);
        else 
            for (int j = 0; j < c; j++) 
                out[idx + j] = 0;
}

int main(int argc, char *argv[]) {
    if(argc < 5) {
        printf("Wrong arguments!\n");
        return -1;
    }

    struct timeval after, before, result;
    gettimeofday(&before, NULL);

    char *DIR_IMG_INPUT, *DIR_IMG_OUTPUT;
    int KERNEL_SIZE, THREADS, verbose;
    double sigma;

    DIR_IMG_INPUT = argv[1];
    DIR_IMG_OUTPUT = argv[2];

    KERNEL_SIZE = 3;
    sscanf(argv[3], "%d", &KERNEL_SIZE);
    if(KERNEL_SIZE % 2 == 0) {
        printf("The kernel size must be odd");
        return -1;
    }

    sigma = SIGMA;
    if(argv[4] != 0) sscanf(argv[4], "%lf", &sigma);

    if(argv[5] != NULL) {
        sscanf(argv[5], "%d", &verbose);
        if(verbose != 1)
            verbose = 0;
    }

    int deviceCount = 0;
    CHECK(cudaGetDeviceCount(&deviceCount));

    if (deviceCount == 0) {
        printf("There are no available device(s) that support CUDA\n");
        return -1;
    }

    if(verbose) printf("Detected %d CUDA Capable device(s)\n", deviceCount);

    cudaSetDevice(0);
    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, 0);

    int coresPerMP = _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor);
    int multiProcessors = deviceProp.multiProcessorCount;

    if(verbose)
      printf("%d Multiprocessors, %d CUDA Cores/MP | %d CUDA Cores\nMaximum number of threads per block: %d\n",
             deviceProp.multiProcessorCount,
             _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor),
             _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor) * deviceProp.multiProcessorCount,
             deviceProp.maxThreadsPerBlock);
    
    int blocksPerGrid, threadsPerBlock;
    int kernel_cells = KERNEL_SIZE * KERNEL_SIZE;

    double h_kernel[KERNEL_SIZE][KERNEL_SIZE];
    double *d_kernel;

    CHECK(cudaMalloc((void **) &d_kernel, kernel_cells * sizeof(double)));

    double *d_sum;
    CHECK(cudaMalloc((void **) &d_sum, sizeof(double)));

    threadsPerBlock = MIN(coresPerMP, kernel_cells);
    blocksPerGrid = floor(kernel_cells / threadsPerBlock) + 1;

    generateGaussianKernel<<<blocksPerGrid, threadsPerBlock>>>((double *) d_kernel, d_sum, KERNEL_SIZE, sigma);

    CHECK(cudaMemcpy(h_kernel, d_kernel, kernel_cells * sizeof(double), cudaMemcpyDeviceToHost));

    if(verbose)
      printf("Kernel computed in %d threads in %d blocks\n.", threadsPerBlock, blocksPerGrid);

    cudaFree(d_sum);

    unsigned char *h_data;
    int width, height, channels;
    h_data = stbi_load(DIR_IMG_INPUT, &width, &height, &channels, STBI_default);

    if (h_data != NULL) {
        if(verbose) printf("\nImage dimensions: (%dpx, %dpx) and %d channels.\n", width, height, channels);

        unsigned char *h_output_image, *d_data, *d_output_image;
        CHECK(cudaMalloc((void **) &d_output_image, width * height * channels * sizeof(unsigned char)));
        CHECK(cudaMalloc((void **) &d_data, width * height * channels * sizeof(unsigned char)));
        CHECK(cudaMemcpy(d_data, h_data, width * height * channels * sizeof(unsigned char), cudaMemcpyHostToDevice));
        
        h_output_image = (unsigned char*) malloc(width * height * channels * sizeof(unsigned char));
        if(h_output_image == NULL) {
            printf("Error trying to allocate memory space");
            free(h_output_image);
            stbi_image_free(h_data);
            return -1;
        }

        threadsPerBlock = MIN(coresPerMP * 2, width * height);
        blocksPerGrid = floor(width * height / threadsPerBlock) + 1;

        applyFilter<<<blocksPerGrid, threadsPerBlock>>>(d_data, d_output_image, d_kernel, width, height, channels, KERNEL_SIZE);

        if(verbose)
          printf("Filter applied with %d threads in %d blocks\n.", threadsPerBlock, blocksPerGrid);

        CHECK(cudaMemcpy(h_output_image, d_output_image, width * height * channels * sizeof(unsigned char), cudaMemcpyDeviceToHost));

        CHECK(cudaFree(d_kernel));
        CHECK(cudaFree(d_output_image));
        CHECK(cudaFree(d_data));

        if (!stbi_write_png(DIR_IMG_OUTPUT, width, height, channels, h_output_image, width * channels))
            printf("Image cannot be created");
        else
            if(verbose) printf("Image created");

        free(h_output_image);
        free(h_data);
    } else {
        printf("Error loading the image");
    }

    gettimeofday(&after, NULL);
    timersub(&after, &before, &result);

    if(verbose) printf("\nTime elapsed: %ld.%06ld\n", (long int) result.tv_sec, (long int) result.tv_usec);
    else
        printf("%ld.%06ld\n", (long int)result.tv_sec, (long int)result.tv_usec);

    stbi_image_free(h_data);
}