#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define DIR_IMG_INPUT "img/test.png"
#define DIR_IMG_OUTPUT "out/out_test.png"

int main() {
    int x,y,n;
    unsigned char *data = stbi_load(DIR_IMG_INPUT, &x, &y, &n, STBI_grey);

    if (data == NULL) {
        printf("Error loading the image");
    } else {
        printf("Image loaded: Width %dpx, Height %dpx and %d channels\n", x, y, n);

        printf("Iterating over the array: ");
        for (int i = 0; i < 10; i++)
            printf("%u ", data[i]);
        
        stbi_write_png(DIR_IMG_OUTPUT, x, y, n, data, x);
    }

    stbi_image_free(data);
}