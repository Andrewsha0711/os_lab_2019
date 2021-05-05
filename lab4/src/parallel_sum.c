#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pthread.h>
#include "sum.h"
#include "utils.h"
#include <getopt.h> // для опций
#include <sys/time.h> // для измерений времени

void *ThreadSum(void *args) { // для нахождения суммы в потоке на заданном промежутке
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)Sum(sum_args);
}

int main(int argc, char **argv) {

  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;
  pthread_t threads[threads_num];
  /*
   *  TODO:
   *  threads_num by command line arguments
   *  array_size by command line arguments
   *	seed by command line arguments
   */
  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"threads_num", required_argument, 0, 0},
                                      {"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) 
        {
          case 0:
            threads_num = atoi(optarg);
            if (threads_num <= 0){
              printf("threads_num is a positive number\n");
              return 1;
            }
            break;
          case 1:
            seed = atoi(optarg);
            if(seed <= 0){
              printf("seed is a positive number\n");
              return 1;
            }
            break;
          case 2:
            array_size = atoi(optarg);
            if(array_size <= 0){
              printf("array_size is a positive number\n");
              return 1;
            }
            break;
          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case '?': 
        break;
      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (threads_num == 0 || seed == 0 || array_size == 0) {
    printf("Usage: %s --threads_num \"num\" --seed \"num\" --array_size \"num\" \n",
           argv[0]);
    return 1;
  }
  
  // выделение памяти и генерация массива
  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  for(int i=0; i<array_size;i++)
  printf("%d ", array[i]);
  printf("\n");


  // засекаем таймер после генерации
  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  struct SumArgs args[threads_num]; // наборы аргументов (параметров массива) для потоков
  // заполняем наборы аргументов
  for (uint32_t i = 0; i < threads_num; i++) 
  {
    args[i].array = array;
    args[i].begin = i*(array_size/threads_num);
    if (i == threads_num - 1)
      args[i].end = array_size;
    else
      args[i].end = (i+1)*(array_size/threads_num);
  }
  // создаем потоки
  for (uint32_t i = 0; i < threads_num; i++) 
  {
    if (pthread_create(&threads[i],    // адрес для хранения идентификатора создаваемого потока типа pthread_t
                       NULL,           // бестиповый указатель атрибутов потока. Если этот аргумент равен NULL, то поток создается с атрибутами по умолчанию. 
                       ThreadSum,      // указатель на потоковую void * функцию
                       (void *)&args[i])) // бестиповый указатель, содержащий аргументы потока
                       // возвращает 0 при удачном создании потока
    {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }
  
  // ожидание потоков, вычисление общей суммы
  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    int _sum = 0;
    pthread_join(threads[i], (void **)&_sum); // ожидание выполнения потока (идентификатор + данные возвращаемые потоком через return)
    total_sum += _sum; // суммирование частных сум
  }
  
  // фиксируем время
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);
  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  // освобождаем память, выводим результат
  free(array);
  printf("Total: %d %f\n", total_sum,elapsed_time);
  return 0;
}
