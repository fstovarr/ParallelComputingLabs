#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <sys/time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb/stb_image_write.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

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
            res = (double)(exp(-(x * x + y * y) / two_sigma_sq) / (two_sigma_sq * M_PI));//formula general para kernel gaussiano
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
                v = *(kernel + (m  + kernel_pad) * kernel_size + (n + kernel_pad));//valor en el kernel
                total += v * in[(i + l) +  (m * w * channels) + (n * channels )];// acumulado del producto punto a punto del kernel y la imagen original
            }

        out[i + l] = total;
    }
}

// se aplica el filtro de forma secuencial para cada pixel
// here h is the span of our chunk
// from and to are used to determine the start and end of processing.
void applyFilter(unsigned char *in, unsigned char *out, int w, int h, int c, double* kernel, int kernel_size, long long int from, long long int to) {
    int kernel_pad = kernel_size / 2;
    size_t size = w * h;

    for (long long int i = from; i < to; i += c)
        if(i > kernel_pad * w * c && // Top
            i < (size * c - kernel_pad * w * c) && // Bottom
            i % (w * c) >= kernel_pad * c && // Left
            i % (w * c) < (w * c - kernel_pad * c)) // Right
            calculatePixel(in, out, i, w, h, c, kernel, kernel_size);//solo se invoca si el pixel tiene un contorno coherente con el size del kernel
        else
            for (int j = 0; j < c; j++)
                out[i + j] = 0;
}

int main(int argc, char *argv[]) {
    if(argc > 5) {
        printf("Wrong arguments!\n");
        return -1;
    }
    //struct timeval after, before, result;
    //gettimeofday(&before, NULL);

    //los argumentos son: direccion de entrada, direccion de salida, size del kernel, numero de hilos
    char *DIR_IMG_INPUT = argv[1];
    char *DIR_IMG_OUTPUT = argv[2];
    int KERNEL_SIZE = 3; int kernel_size_tag = 0;
    if(argv[3] != NULL) sscanf(argv[3], "%d", &KERNEL_SIZE);
    int verbose = 0;
    if(argv[4] != NULL) sscanf(argv[4], "%d", &verbose);


    int iam, n_spawned;

    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n_spawned);
    MPI_Comm_rank(MPI_COMM_WORLD, &iam);

    double kernel[KERNEL_SIZE*KERNEL_SIZE]; int kernel_tag = 1;
    if(iam == 0) { // This executes on root/main node
        generateGaussianKernel(kernel, KERNEL_SIZE);
        for(int i = 1; i < n_spawned; i++)
            MPI_Send(kernel, KERNEL_SIZE*KERNEL_SIZE, MPI_DOUBLE, i, kernel_tag, MPI_COMM_WORLD);
    } else { // Other nodes
        MPI_Recv(kernel, KERNEL_SIZE*KERNEL_SIZE, MPI_DOUBLE, 0, kernel_tag, MPI_COMM_WORLD, &status);
    }

    int chunk_start, chunk_end; int chunk_start_tag = 2, chunk_end_tag = 3;
    int width, height, channels; int width_tag = 4, height_tag = 5, channels_tag = 6;
    unsigned char *chunk_data, *chunk_data_out; int chunk_data_tag = 7;
    int chunk_lines; int chunk_lines_tag = 8;

    if(iam == 0) {
        // load the image in root node
        unsigned char *data = stbi_load(DIR_IMG_INPUT, &width, &height, &channels, STBI_default);
        if (data != NULL) {
            if(verbose) printf("Image dimensions: (%dpx, %dpx) and %d channels.\n", width, height, channels);

            int boundary = KERNEL_SIZE / 2;
            if(n_spawned > height) n_spawned = height; // max threads = # of rows of the image
            int min_lines_per_block = height / n_spawned; // minimum lines allowed per block
            int extra = height - (min_lines_per_block*n_spawned), offset = 0; // used to better distribute the data

            int malloc_one = 0, malloc_two = 0;

            for(int i = n_spawned - 1 ; i >= 0 ; i--){ // backwards to keep the first one on the root node
                int lines_per_block = min_lines_per_block;
                if (i < extra)
                    lines_per_block++;
                chunk_lines = lines_per_block + (2 * boundary);
                
                // we use this to avoid overhead when dealing with our possible chunk sizes
                if (i < extra) {
                    if(malloc_one == 0){ // malloc once when chunk has size (min_lines_per_block + 1)
                        chunk_data = malloc(chunk_lines * width * channels);
                        malloc_one = 1;
                    }
                } else {
                    if(malloc_one == 1) free(chunk_data); // free if already allocated
                    if(malloc_two == 0){ // malloc once when chunk has size (min_lines_per_block)
                        chunk_data = malloc(chunk_lines * width * channels);
                        malloc_two = 0;
                    }
                    offset = extra;
                }

                // lines that limit the chunk + boundaries from the image
                int start_line = offset + (i * lines_per_block) - boundary;
                int end_line = (MIN(offset + (i * lines_per_block) + lines_per_block, height)) + boundary;
                int top_bound = boundary, bottom_bound = boundary;
                //printf("%03d - FROM: %d - TO: %d, BUCKET: %d\n", i ,start_line + boundary, end_line - boundary, end_line - start_line - 2*boundary);
                if(start_line < 0){
                    top_bound = boundary + start_line;
                    start_line = 0;
                }
                if(end_line > height){
                    bottom_bound = boundary - (end_line - height);
                    end_line = height;
                }


                // lines that limit our outer boundary on chunk_data (aka, where we copy image chunk + boundaries)
                int chunk_cp_start_line = (i == n_spawned - 1) ? chunk_lines - (end_line - start_line) : 0 ;
                int chunk_cp_end_line = chunk_cp_start_line + (end_line - start_line);

                // lines that limit our inner boundary on chunk_data
                int chunk_start_line = chunk_cp_start_line + top_bound;
                int chunk_end_line = chunk_cp_end_line - bottom_bound;


                memcpy(chunk_data + (chunk_cp_start_line * width * channels),
                    data + (start_line * width * channels),
                    (chunk_cp_end_line - chunk_cp_start_line) * width * channels
                );
                chunk_start = chunk_start_line * width * channels;
                chunk_end = chunk_end_line * width * channels;

                if(i != 0) { // keep first one locally, share the others with the slave nodes
                    MPI_Send(&KERNEL_SIZE, 1, MPI_INT, i, kernel_size_tag, MPI_COMM_WORLD);
                    MPI_Send(&width, 1, MPI_INT, i, width_tag, MPI_COMM_WORLD);
                    MPI_Send(&channels, 1, MPI_INT, i, channels_tag, MPI_COMM_WORLD);
                    MPI_Send(&chunk_start, 1, MPI_INT, i, chunk_start_tag, MPI_COMM_WORLD);
                    MPI_Send(&chunk_end, 1, MPI_INT, i, chunk_end_tag, MPI_COMM_WORLD);
                    MPI_Send(&chunk_end, 1, MPI_INT, i, chunk_end_tag, MPI_COMM_WORLD);
                    MPI_Send(&chunk_lines, 1, MPI_INT, i, chunk_lines_tag, MPI_COMM_WORLD);

                    MPI_Send(chunk_data, 
                        chunk_lines * width * channels,
                        MPI_UNSIGNED_CHAR, i, chunk_data_tag, MPI_COMM_WORLD);
                        
                }
                //printf("start: %d - end: %d\n", chunk_start, chunk_end);

                // process first chunk locally
                chunk_data_out = malloc(chunk_lines * width * channels);
                applyFilter(chunk_data, chunk_data_out, width, chunk_lines, channels, kernel, KERNEL_SIZE, chunk_start, chunk_end);
                free(chunk_data);
            }
        } else {
            if(verbose)printf("Error loading the image");
        }
        stbi_image_free(data);
    } else {
        // Receive chunk and relevant data
        MPI_Recv(&KERNEL_SIZE, 1, MPI_INT, 0, kernel_size_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&width, 1, MPI_INT, 0, width_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&channels, 1, MPI_INT, 0, channels_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&chunk_start, 1, MPI_INT, 0, chunk_start_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&chunk_end, 1, MPI_INT, 0, chunk_end_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&chunk_lines, 1, MPI_INT, 0, chunk_lines_tag, MPI_COMM_WORLD, &status);
        
        chunk_data = malloc(chunk_lines * width * channels); // it might not be allocated
        chunk_data_out = malloc(chunk_lines * width * channels);
        MPI_Recv(chunk_data, chunk_lines * width * channels, MPI_UNSIGNED_CHAR, 0, chunk_data_tag, MPI_COMM_WORLD, &status);

        // Process chunk
        applyFilter(chunk_data, chunk_data_out, width, chunk_lines, channels, kernel, KERNEL_SIZE, chunk_start, chunk_end);
        free(chunk_data);
    }

    unsigned char *output_image = NULL;
    if(iam == 0){
        output_image = malloc(width * height * channels);
    }

    MPI_Gather(chunk_data_out + chunk_start, (chunk_end - chunk_start), MPI_UNSIGNED_CHAR,
        output_image, (chunk_end - chunk_start), MPI_UNSIGNED_CHAR,
        0, MPI_COMM_WORLD);
    free(chunk_data_out);
    
    if(iam == 0) {
        if(output_image == NULL) {
            if(verbose)printf("Error trying to allocate memory space");
        }
        if(!stbi_write_png(DIR_IMG_OUTPUT, width, height, channels, output_image, width * channels))
            if(verbose)printf("Image cannot be created");
        free(output_image);
    }



    //gettimeofday(&after, NULL);
    //timersub(&after, &before, &result);
    //printf("%ld.%06ld\n", (long int) result.tv_sec, (long int) result.tv_usec);

    MPI_Finalize();
    return 0;
}

