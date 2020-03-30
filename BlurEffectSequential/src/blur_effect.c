#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

#define DIR_IMG_INPUT "img/test.png"
#define DIR_IMG_OUTPUT "out/out_test.png"

int main() {
    int width, height, channels;
    unsigned char *data = stbi_load(DIR_IMG_INPUT, &width, &height, &channels, STBI_default);

    if (data != NULL) {
        size_t size = width * height * channels;
        printf("Image dimensions: (%dpx, %dpx) and %d channels.\nVector size: %ld\n", width, height, channels, size);

        unsigned char *grey_image = malloc(width * height * 1) ;

        printf("Iterating over the array: ");
        for (int i = 0, j = 0; i < size - 4; i += 3, j++) {
            grey_image[j] = (data[i] + data[i + 1] + data[i + 2]) / 3.0;
        }

        stbi_write_png(DIR_IMG_OUTPUT, width, height, 1, grey_image, width * 1);
    } else {
        printf("Error loading the image");
    }

    stbi_image_free(data);
}