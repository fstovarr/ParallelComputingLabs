#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


void generateGaussianKernel(double *k, int size) {
    double sigma = 2.0;
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
        }

    for (int x = 0; x < size; x++)
        for (int y = 0; y < size; y++) {
            res = *(k + x * size + y) / sum;
            memcpy(k + x * size + y, &res, sizeof(res));
        }
}

unsigned char *blurPx(unsigned char *mat, double *m3, int k, int channels){
  unsigned char *px = (unsigned char *)malloc(sizeof(unsigned char) * channels);
  int i, j, l, id;
  for (i = 0; i < channels; i++) px[i] = 0;

  for(i = 0; i < k; i++) {
    for(j = 0; j < k;j++){
      id = (j*k + i);
      for(l = 0; l < channels; l++){
        px[l] += (mat[id * channels + l] * m3[id]);
      }
    }
  }
  return px;
}

int main(int argc, char **argv) {
  //int kern_size = atoi(argv[3]), threads = atoi(argv[4]);
  int k = atoi(argv[3]);
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

      double *m3 = (double *)malloc(sizeof(double) * k * k);
      generateGaussianKernel(m3, k);
      unsigned char *mat = (unsigned char *)malloc(sizeof(unsigned char) * k * k * channels);
      
      for(id = 0; id < img_size ; id += channels){
        yy = ((id / channels) / x) - ((k - 1) / 2);
        xx = ((id / channels) % x) - ((k - 1) / 2);
        
        for(i = 0; i < k; i++){
          for(j = 0; j < k; j++) {
            id_mat = (i * k + j) * channels;
            if(yy + j < 0 || yy + j >= y || xx + i < 0 || xx + i >= x){
              for(l = 0 ; l < channels; l++) mat[id_mat + l] = 0;
            } else {
              decoded = ((yy + j) * x * channels) + ((xx + i) * channels);
              for(l = 0;l < channels; l++) mat[id_mat + l] = data[decoded + l];
            }
          }
        }
        
        unsigned char *px = blurPx(mat, m3, k, channels);
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
