#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include<sys/time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

// Pool based on https://nachtimwald.com/2019/04/12/thread-pool-in-c/

struct tpool;
typedef struct tpool tpool_t;

struct func_params;
typedef struct func_params func_params_t;

tpool_t *tpool_create(size_t num);
void tpool_destroy(tpool_t *tm);

bool tpool_add_work(tpool_t *tm, void *arg);
void tpool_wait(tpool_t *tm);

void *parallelFunction(void *args);


// -------------- WORK QUEUE.  - linked list ------------
struct tpool_work {
  void              *arg; // ARGS
  struct tpool_work *next; // NEXT ELEMENT
};
typedef struct tpool_work tpool_work_t;

// ---------------------- POOL ----------------------------
struct tpool {
  tpool_work_t    *work_first;
  tpool_work_t    *work_last; 
  pthread_mutex_t  work_mutex; // all locking
  pthread_cond_t   work_cond; // signals the threads that there is work left to do
  pthread_cond_t   working_cond; // signals that there are no threads processing 
  size_t           working_cnt; // Number of current active threads
  size_t           thread_cnt; // Saves the number of threads for the program
  bool             stop; // stops the threads
};

// -------------- HELPERS to create/destroy work objects --------------
static tpool_work_t *tpool_work_create(void *arg) {
  tpool_work_t *work;

  work = malloc(sizeof(*work));
  work->arg = arg;
  work->next = NULL;
  return work;
}

static void tpool_work_destroy(tpool_work_t *work){
  if (work == NULL)
    return;
  free(work);
}

// pulls work form queue in order to be processed
// also keeps track of work first and work last
static tpool_work_t *tpool_work_get(tpool_t *tm) {
  tpool_work_t *work;

  if (tm == NULL)
    return NULL;

  work = tm->work_first;
  if (work == NULL)
    return NULL;

  if (work->next == NULL) {
    tm->work_first = NULL;
    tm->work_last  = NULL;
  } else {
    tm->work_first = work->next;
  }
  return work;
}


//--------------------- WORKER FUNCTION ------------------------

static void *tpool_worker(void *arg) {
  tpool_t      *tm = arg;
  tpool_work_t *work;

  while (1) {
    // >> pull
    pthread_mutex_lock(&(tm->work_mutex));

    while (tm->work_first == NULL && !tm->stop)
      pthread_cond_wait(&(tm->work_cond), &(tm->work_mutex));

    if (tm->stop)
      break;

    work = tpool_work_get(tm);
    tm->working_cnt++;
    pthread_mutex_unlock(&(tm->work_mutex));
    // << end pull

    if (work != NULL) {
      parallelFunction(work->arg);
      tpool_work_destroy(work);
    }

    // >> sync
    pthread_mutex_lock(&(tm->work_mutex));
    tm->working_cnt--;
    if (!tm->stop && tm->working_cnt == 0 && tm->work_first == NULL)
      pthread_cond_signal(&(tm->working_cond));
    pthread_mutex_unlock(&(tm->work_mutex));
    // << end sync 
  }

  tm->thread_cnt--;
  pthread_cond_signal(&(tm->working_cond));
  pthread_mutex_unlock(&(tm->work_mutex));
  return NULL;
}

// ----------------- POOL CREATE/DESTROY ------------------------
tpool_t *tpool_create(size_t num) {
  tpool_t *tm;
  pthread_t thread;
  size_t i;

  if (num == 0)
    num = 2;

  tm = calloc(1, sizeof(*tm));
  tm->thread_cnt = num;

  pthread_mutex_init(&(tm->work_mutex), NULL);
  pthread_cond_init(&(tm->work_cond), NULL);
  pthread_cond_init(&(tm->working_cond), NULL);

  tm->work_first = NULL;
  tm->work_last  = NULL;

  for (i=0; i<num; i++) {
    pthread_create(&thread, NULL, tpool_worker, tm);
    pthread_detach(thread);
  }

  return tm;
}

// destroy pool
void tpool_destroy(tpool_t *tm) {
  tpool_work_t *work;
  tpool_work_t *work2;

  if (tm == NULL)
    return;

  pthread_mutex_lock(&(tm->work_mutex));
  work = tm->work_first;
  while (work != NULL) {
    work2 = work->next;
    tpool_work_destroy(work);
    work = work2;
  }

  tm->stop = true;
  pthread_cond_broadcast(&(tm->work_cond));
  pthread_mutex_unlock(&(tm->work_mutex));

  tpool_wait(tm);

  pthread_mutex_destroy(&(tm->work_mutex));
  pthread_cond_destroy(&(tm->work_cond));
  pthread_cond_destroy(&(tm->working_cond));

  free(tm);
}

// enqueue work
bool tpool_add_work(tpool_t *tm, void *arg) {
  tpool_work_t *work;

  if (tm == NULL)
    return false;

  work = tpool_work_create(arg);
  if (work == NULL)
    return false;

  pthread_mutex_lock(&(tm->work_mutex));
  if (tm->work_first == NULL) {
    tm->work_first = work;
    tm->work_last  = tm->work_first;
  } else {
    tm->work_last->next = work;
    tm->work_last       = work;
  }

  pthread_cond_broadcast(&(tm->work_cond));
  pthread_mutex_unlock(&(tm->work_mutex));

  return true;
}

// wait for processing

void tpool_wait(tpool_t *tm)
{
    if (tm == NULL)
        return;

    pthread_mutex_lock(&(tm->work_mutex));
    while (1) {
        if ((!tm->stop && tm->working_cnt != 0) || (tm->stop && tm->thread_cnt != 0)) {
            pthread_cond_wait(&(tm->working_cond), &(tm->work_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tm->work_mutex));
}


// --------------------------------------------------------------------------------

struct func_params {
  unsigned char *in;
  unsigned char *out;
  int i, w, h, channels;
  double *kernel;
  int kernel_size;
  int from, to;
};

func_params_t *createParams(unsigned char *in, unsigned char *out, int w, int h, int c, double *kernel, int kernel_size, int from, int to){
  func_params_t *args; 
  args = calloc(1,sizeof(*args));
  args->in = in;
  args->out = out;
  args->w = w;
  args->h = h;
  args->channels = c;
  args->kernel = kernel;
  args->kernel_size = kernel_size;
  args->from = from;
  args->to = to;
  return args;
}


// http://pages.stat.wisc.edu/~mchung/teaching/MIA/reading/diffusion.gaussian.kernel.pdf.pdf
void generateGaussianKernel(double *k, int size) {
  double sigma = (size - 1.0) / 6.0;
  // double sigma = 30;
  double two_sigma_sq = 2.0 * sigma * sigma;

  double sum = 0.0;
  double res = 0.0;
  int mid_size = size / 2;

  for (int x = -mid_size; x <= mid_size; x++)
    for (int y = -mid_size; y <= mid_size; y++){
      int idx = (x + mid_size) * size + y + mid_size;
      // double r = sqrt();
      res = (double)(exp(-(x * x + y * y) / two_sigma_sq) / (two_sigma_sq * M_PI));
      memcpy(k + idx, &res, sizeof(res));
      sum += *(k + idx);
    }

  for (int x = 0; x < size; x++)
    for (int y = 0; y < size; y++){
      res = *(k + x * size + y) / sum;
      memcpy(k + x * size + y, &res, sizeof(res));
    }
}


void calculatePixel(unsigned char *in, unsigned char *out, int i, int w, int h, int channels, double *kernel, int kernel_size){
  int kernel_pad = kernel_size / 2;
  double v = 0.0, total = 0.0;

  for (int l = 0; l < channels; l++){
    total = 0.0;
    for (int m = -kernel_pad; m <= kernel_pad; m++)
      for (int n = -kernel_pad; n <= kernel_pad; n++){
        v = *(kernel + (m + kernel_pad) * kernel_size + (n + kernel_pad));
        total += v * in[(i + l) + (m * w * channels) + (n * channels)];
      }
    out[i + l] = total;
  }
}


void *parallelFunction(void *args){
  func_params_t *params = args;

  int w = params->w, h = params->h, c = params->channels;
  
  int kernel_pad = params->kernel_size / 2;
  size_t size = w * h * c;

  for(int i = params->from; i < params->to; i++)
    if (i >= kernel_pad * w * c &&              // Top
        i < (size - kernel_pad * w * c) &&      // Bottom
        i % (w * c) >= kernel_pad * c &&        // Left
        i % (w * c) < (w * c - kernel_pad * c)) // Right
      calculatePixel(params->in, params->out, i, w, h, c, params->kernel, params->kernel_size);
    else
      for (int j = 0; j < c; j++)
        params->out[i + j] = 0;
  
  return NULL;
}

int applyFilter(unsigned char *in, unsigned char *out, int w, int h, int c, double *kernel, int kernel_size, int bucket_size, int num_threads){
  //input, output, width, height, channels, kernels, kernel_size
  size_t size = w * h;
  if(bucket_size >= w*h || bucket_size <=0) bucket_size = 1; // default  

  tpool_t *tm;
  int i;

  tm = tpool_create(num_threads);


  for (i = bucket_size; i < size; i += bucket_size){
    func_params_t *args = createParams(in, out, w, h, c, kernel, kernel_size, (i - bucket_size) * c ,i * c);
    //printf("%d - %d\n", i-bucket_size,i);
    tpool_add_work(tm, args);
  }
  
  //last case
  if(i >= size){
    func_params_t *args = createParams(in, out, w, h, c, kernel, kernel_size, (i-bucket_size)*c , size*c);
    //printf("%d - %d\n", i,size);
    tpool_add_work(tm, args);
  }

  tpool_wait(tm);
  tpool_destroy(tm);
  return 0;
}


int main(int argc, char **argv){
  if(argc != 5){
    printf("Wrong arguments \n");
    return -1;
  }

  struct timeval after, before, result;
  gettimeofday(&before, NULL);
  
  char *DIR_IMG_INPUT = argv[1];
  char *DIR_IMG_OUTPUT = argv[2];
  int KERNEL_SIZE = atoi(argv[3]);
  int N_THREADS = atoi(argv[4]);
  int bucket_size = 255; // this to avoid false sharing, check cache size to tune
  double sigma = 15;
  int verbose = 0;
  if(argv[5] != NULL) sigma = atof(argv[5]);
  if(argv[6] != NULL) verbose = atoi(argv[6]);

  double kernel[KERNEL_SIZE][KERNEL_SIZE];
  generateGaussianKernel(kernel, KERNEL_SIZE);

  int width, height, channels;
  unsigned char *data = stbi_load(DIR_IMG_INPUT, &width, &height, &channels, STBI_default);

  if(data != NULL){
    if(verbose) printf("Image dimensions: (%dpx, %dpx) and %d channels.\n", width, height, channels);

    unsigned char *output_image = malloc(width * height * channels);
    if(output_image == NULL) {
      if(verbose) printf("Error trying to allocate memory space");
      stbi_image_free(data);
      return -1;
    }

    applyFilter(data, output_image, width, height, channels, kernel, KERNEL_SIZE, bucket_size, N_THREADS);

    if(!stbi_write_png(DIR_IMG_OUTPUT, width, height, channels, output_image, width * channels))
      if(verbose) printf("Image cannot be created");
    free(output_image);
  } else {
    if(verbose) printf("Error loading the image");
  }

  gettimeofday(&after, NULL);
  timersub(&after, &before, &result);
  printf("%ld.%06ld\n", (long int) result.tv_sec, (long int) result.tv_usec);

  stbi_image_free(data);
}
