#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pthread.h>
#include <getopt.h> // для опций
#include <sys/time.h> // для измерений времени

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

struct FunArgs {
  int begin; // идентификатор начального элемента отрезка
  int end; // идентификатор конечного элемента отрезка
  int* res;
  int k;
};

void Fun(const struct FunArgs *args) 
{
  for (int i = args->begin; i < args->end+1; i++)
  {
      if(i <= args->k) // проверка связанная с возможной регулировкой шага
      {
        pthread_mutex_lock(&mut);
        *(args->res) = *(args->res)*i;
        pthread_mutex_unlock(&mut);
      }
  }
}


int main(int argc, char **argv) {

  uint32_t threads_num = 0;
  uint32_t k = 0;
  uint32_t mod = 0;
  pthread_t threads[threads_num];

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"threads_num", required_argument, 0, 0},
                                      {"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
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
            k = atoi(optarg);
            if(k <= 0){
              printf("k is a positive number\n");
              return 1;
            }
            break;
          case 2:
            mod = atoi(optarg);
            if(mod <= 0){
              printf("mod is a positive number\n");
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

  if (threads_num == 0 || k == 0 || mod == 0) {
    printf("Usage: %s --threads_num \"num\" --k \"num\" --mod \"num\" \n",
           argv[0]);
    return 1;
  }
  
  if (threads_num > k) threads_num = k; // регулировка количества потоков
  // засекаем таймер 
  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  struct FunArgs args[threads_num]; // наборы аргументов для потоков
  int count = (k/threads_num);
  if (count*threads_num < k) count +=1; // регулировка шага
  int* res = malloc(sizeof(int));
  *res = 1;

    // заполняем наборы аргументов
  for (uint32_t i = 0; i < threads_num; i++) 
  {
    args[i].begin = (i*count)+1;
    args[i].end = args[i].begin + count -1;
    args[i].res = res; // накапливаемый факториал
    args[i].k = k; // начальное число для проверки множителей
  }
  // создаем потоки
  for (uint32_t i = 0; i < threads_num; i++) 
  {
    if (pthread_create(&threads[i],    // адрес для хранения идентификатора создаваемого потока типа pthread_t
                       NULL,           // бестиповый указатель атрибутов потока. Если этот аргумент равен NULL, то поток создается с атрибутами по умолчанию. 
                       (void *)Fun,      // указатель на потоковую void * функцию
                       (void *)&args[i])) // бестиповый указатель, содержащий аргументы потока
                       // возвращает 0 при удачном создании потока
    {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }
  
  // ожидание потоков
  for (uint32_t i = 0; i < threads_num; i++) {
    pthread_join(threads[i], NULL); // ожидание выполнения потока (идентификатор + данные возвращаемые потоком через return)
  }
  
  // фиксируем время
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);
  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  // освобождаем память, выводим результат
  printf("Result: %d\n", *res);
  return 0;
}
