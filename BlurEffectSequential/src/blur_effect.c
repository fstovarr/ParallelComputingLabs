#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

// #define DIR_IMG_INPUT "img/favicon.png"
#define DIR_IMG_INPUT "img/test2.jpg"
#define DIR_IMG_OUTPUT "out/out_test.jpg"

const int kernel_pad = 2;
const double kernel[5][5] = { 
    {1.0 / 256,  4.0 / 256,  6.0 / 256,  4.0 / 256, 1.0 / 256}, 
    {4.0 / 256, 16.0 / 256, 24.0 / 256, 16.0 / 256, 4.0 / 256}, 
    {6.0 / 256, 24.0 / 256, 36.0 / 256, 24.0 / 256, 6.0 / 256}, 
    {4.0 / 256, 16.0 / 256, 24.0 / 256, 16.0 / 256, 4.0 / 256}, 
    {1.0 / 256,  4.0 / 256,  6.0 / 256,  4.0 / 256, 1.0 / 256} 
};

// const int kernel_pad = 1;
// const double kernel[3][3] = {
//     { 0.0625,   0.125,  0.0625 },
//     { 0.1250,   0.25,  0.125 },
//     { 0.0625,   0.125,  0.0625 }
// };

void calculatePixel(unsigned char *in, unsigned char *out, int i, int w, int h, int channels) {
    for (int l = 0; l < channels; l++) {
        double total = 0.0;
        for (int m = -kernel_pad/2; m <= kernel_pad; m++)
            for (int n = -kernel_pad/2; n <= kernel_pad; n++)
                total += kernel[m + kernel_pad][n + kernel_pad] * in[(i + l) + m * channels + n * channels];
        out[i + l] = total;
    }
}

void applyFilter(unsigned char *in, unsigned char *out, int w, int h, int c) {
    size_t size = w * h * c;
    for (int i = 0; i < size; i += c)
        if(i > kernel_pad * w * c && 
            i < (size - kernel_pad * w * c) && 
            i % (w * c) >= kernel_pad && 
            i % (w * c) != (kernel_pad * w * c - c))
            calculatePixel(in, out, i, w, h, c);
        else
            for (int j = 0; j < c; j++)
                out[i + j] = 0;
}

int main() {
    int width, height, channels;
    unsigned char *data = stbi_load(DIR_IMG_INPUT, &width, &height, &channels, STBI_rgb);
    channels = 3;

    if (data != NULL) {
        size_t size = width * height * 3;
        printf("Image dimensions: (%dpx, %dpx) and %d channels.\n", width, height, channels);

        unsigned char *output_image = malloc(width * height * channels);
        applyFilter(data, output_image, width, height, channels);

        stbi_write_png(DIR_IMG_OUTPUT, width, height, channels, output_image, width * channels);
    } else {
        printf("Error loading the image");
    }

    stbi_image_free(data);
}