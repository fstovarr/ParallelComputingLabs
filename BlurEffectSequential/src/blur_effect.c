#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

unsigned char m3[] = {
  1,2,1,
  2,4,2,
  1,2,1
};

unsigned char *blurPx(unsigned char *mat, int k, int channels){
  unsigned char *px = (unsigned char *)malloc(sizeof(unsigned char) * channels);
  int i, j, id, k2 = k*k;
  for (i = 0; i < channels; i ++){
    px[i] = 0;
  }
  
  for(i = 0; i < k2; i++) {
    id = i*k;
    for (j = 0; j < channels; j ++){
      px[j] += (mat[id + j] * m3[i]) /16;
    }
  }
  return px;
}

int main(int argc, char **argv) {
  //int kern_size = atoi(argv[3]), threads = atoi(argv[4]);
  int k = 3;
  char *in_path = argv[1], *out_path = argv[2];
  int x,y, channels;
  unsigned char *data = stbi_load(in_path, &x, &y, &channels, 3);
  if (data == NULL) {
    printf("Error loading the image");
  } else {
    printf("Image loaded: Width %dpx, Height %dpx and %d channels\n", x, y, channels);
    int img_size = x * y * channels;
    unsigned char *temp_img;
    temp_img = (unsigned char *)malloc(sizeof(unsigned char) * img_size);
    

    if (temp_img == NULL) {
      printf("Couldn't allocate space");
    } else {
      
      int id, id_mat, i, j, l, xx, yy, decoded;
      unsigned char *mat = (unsigned char *)malloc(sizeof(unsigned char) * k * k * channels);
      
      for(id = 0; id < img_size ; id += channels){
        yy = ((id / channels) / x) - ((k - 1) / 2);
        xx = ((id / channels) % x) - ((k - 1) / 2);
        
        for(i = 0; i < k; i++){
          for(j = 0; j < k; j++) {
            id_mat = (i * k + j) * channels;
            if(yy + j < 0 || yy + j >= y || xx + i < 0 || xx + i >= x){
              mat[id_mat] = mat[id_mat + 1] = mat[id_mat + 2] = 0;
            } else {
              decoded = ((yy + j) * x * channels) + ((xx + i) * channels);
              for(l = 0;l < channels; l++){
                mat[id_mat + l] = data[decoded + l];
              }
            }
          }
        }
        
        unsigned char *px = blurPx(mat, k, channels);
        for(l = 0; l < channels; l++){
          temp_img[id + l] = px[l];
        }
      }

      if (!stbi_write_png(out_path, x, y, channels, temp_img, x * channels)){
        printf("Couldn't write image");
      }
      free(temp_img);
    }
  }
  stbi_image_free(data);
}
