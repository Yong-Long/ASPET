#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <omp.h>

#define NUM_THREADS 3

typedef struct
{
  int ID;
  int *a;
  int *b;
  int *result;
} ThreadData;


float s_time () { return (float) clock() / CLOCKS_PER_SEC; }


void printIntArray (int *arr, size_t s)
{
  // size_t s = sizeof(arr) / sizeof(arr[0]) + 1;
  for (size_t i=0; i <= s; i++) { printf("%d, ", arr[i]); }
  printf("\n");
}


// - The function to be executed by threads -
void myThreadFun (void *vargp)
{
  // - Access the parameter arguments -
  ThreadData *data = (ThreadData *)vargp;

  for (int i=0; i < 3; i++) { data->b[i] = data->b[i] + data->ID; }

  printf("\nID: %d\n", data->ID);
  printf("data->a:  \n");
  printIntArray(data->a, sizeof(data->a)/sizeof(data->a[0]));
  printf("data->b:  \n");
  printIntArray(data->b, sizeof(data->b)/sizeof(data->b[0]));

  // - Perform calculation of array -
  for (int j=0; j < 3; j++)
  {
    data->result[j] = data->a[j] + data->b[j];
  }

  printf("data->result:  \n");
  printIntArray(data->result, sizeof(data->result)/sizeof(data->result[0]));
}


int main (int argc, char *argv[])
{
  // - Init array -
  int a[3] = {1, 2, 3};
  int b[3] = {4, 5, 6};

  printf("a:  \n");
  printIntArray(a, sizeof(a)/sizeof(a[0]));
  printf("b:  \n");
  printIntArray(b, sizeof(b)/sizeof(b[0]));

  
  printf("\n====== ====== ====== OMP ====== ====== ====== \n");
  // - Start timing -
  float T[2];
  T[0] = s_time();

  ThreadData *dataOMP = (ThreadData *)malloc(3 * sizeof(ThreadData));
#pragma omp parallel for
  for (int i=0; i < 3; i++)
  {
    dataOMP[i].ID = i;
    dataOMP[i].a  = a;
    dataOMP[i].b  = b;
    dataOMP[i].result = (int *)malloc(3 * sizeof(int));
    printf("\n[OMP]: (%d) \n", i);
    
    myThreadFun(&dataOMP[i]);

    free(dataOMP[i].result);
  }
  // }
  
  // - End timing -
  T[1] = s_time() - T[0];
  printf("\n[Time] OMP: %g sec \n", T[1]);

  free(dataOMP);

  printf("\n====== ====== ====== Normal ====== ====== ====== \n");
  T[0] = s_time();
  
  ThreadData *dataNorm = (ThreadData *)malloc(3 * sizeof(ThreadData));

  for (int i=0; i < 3; i++)
  {
    dataNorm[i].ID = i;
    dataNorm[i].a  = a;
    dataNorm[i].b  = b;
    dataNorm[i].result = (int *)malloc(3 * sizeof(int));
    printf("\n[Iter]: (%d) \n", i);
    
    myThreadFun(&dataNorm[i]);
    
    free(dataNorm[i].result);
  }
  
  T[1] = s_time() - T[0]; 
  printf("\n[Time] Normal: %g sec \n", T[1]);

  free(dataNorm);

  return 0;
}