#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#define NUM_THREADS 3

typedef struct {
  int *a;
  int *b;
  int *result;
} ThreadData;


float s_time() {return (float)clock()/CLOCKS_PER_SEC;}

void printIntArray(int *arr) {
  size_t s = sizeof(arr) / sizeof(arr[0]) + 1;
  for (size_t i=0; i < s; i++) printf("%d, ", arr[i]);
  printf("\n");
}

// The function to be executed by all threads
void *myThreadFun(void *vargp)
{
  // Store the value argument passed to this thread
  ThreadData *data = (ThreadData *)vargp;

  printf("\ndata->a:  \n");
  printIntArray(data->a);
  printf("data->b:  \n");
  printIntArray(data->b);

  for (int j=0; j < 3; j++) {
    data->result[j] = data->a[j] + data->b[j];
  }
  printf("data->result:  \n");
  printIntArray(data->result);
  //pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
  pthread_t  tid[NUM_THREADS];
  // pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  ThreadData *data = (ThreadData *)malloc(3 * sizeof(ThreadData));
  
  float T[4];

  // Init array
  int a[3] = {1, 2, 3};
  int b[3] = {4, 5, 6};

  printf("a:  \n");
  printIntArray(a);
  printf("b:  \n");
  printIntArray(b);

  // ====== ====== ====== Thread ====== ====== ======
  // Start timing
  T[2] = s_time();
  
  // Create threads
  for (int i=0; i < NUM_THREADS; i++) {
    data[i].a = a;
    data[i].b = b;
    data[i].result = (int *)malloc(3 * sizeof(int));
    printf("\nTHREAD: %d\n", i);
    pthread_create(&tid[i], NULL, &myThreadFun, (void *)&data[i]);
  }

  // Join threads
  for (int i=0; i < NUM_THREADS; i++) {
    pthread_join(tid[i], NULL);
    free(data[i].result);
  }

  // End timing
  T[3] = s_time() - T[2];
  printf("\n[Time] thread: %g sec \n", T[3]);

  free(data);

  // ====== ====== ====== Normal ====== ====== ======
  // Start timing
  T[0] = s_time();

  for (int i=0; i < NUM_THREADS; i++) {
    data[i].a = a;
    data[i].b = b;
    data[i].result = (int *)malloc(3 * sizeof(int));
    printf("\nIter: %d\n", i);
    myThreadFun((void *)&data[i]);
  }

  // End timing
  T[1] = s_time() - T[0];
  printf("\n[Time] normal: %g sec \n", T[1]);

  return 0;
}