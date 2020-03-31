#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


unsigned char *blurPx(unsigned char *mat, int k){
  unsigned char *px = (unsigned char *)malloc(sizeof(unsigned char) * 3);
  px[0] = px[1] = px[2] = 0;
  int i, id, k2 = k*k;
  for(i = 0; i < k2; i++) {
    id = i*k;
    px[0] += mat[id]/9;
    px[1] += mat[id + 1]/9;
    px[2] += mat[id + 2]/9;
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
      
      int id, id_mat, i, j, xx, yy, decoded;
      unsigned char *mat = (unsigned char *)malloc(sizeof(unsigned char) * k * k * 3);
      
      for(id = 0; id < img_size ; id += 3){
        yy = ((id / 3) % y) - ((k - 1) / 2);
        xx = ((id / 3) / y) - ((k - 1) / 2);
        
        for(i = 0; i < k; i++){
          for(j = 0; j < k; j++) {
            id_mat = (j * k + i) * 3;
            if(yy + i < 0 || yy + i >= y || xx + j < 0 || xx + j >= x){
              mat[id_mat] = mat[id_mat + 1] = mat[id_mat + 2] = 0;
            } else {
              decoded = ((xx + j) * y * 3) + ((yy + i) * 3);
              mat[id_mat] = data[decoded];
              mat[id_mat + 1] = data[decoded + 1];
              mat[id_mat + 2] = data[decoded + 2];
            }
          }
        }
        
        unsigned char *px = blurPx(mat, k);
        temp_img[id] = px[0];
        temp_img[id + 1] = px[1];
        temp_img[id + 2] = px[2];
        printf("%03d %03d %03d -- %03d %03d %03d\n",temp_img[id], temp_img[id + 1], temp_img[id + 2], data[id], data[id + 1], data[id + 2]);
      }

      if (!stbi_write_png(out_path, x, y, channels, temp_img, x * channels)){
        printf("Couldn't write image");
      }
      free(temp_img);
    }
  }
  stbi_image_free(data);
}
