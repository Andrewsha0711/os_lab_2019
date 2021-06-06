#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* Deadlock чаще всего случается, 
когда поток уже заблокировал ресурс А, 
после чего пытается провести блокировку Б, 
другой же поток заблокировал ресурс Б, 
после чего пытается заблокировать ресурс А.
*/

int sourceA = 0; 
int sourceB = 0; 
pthread_mutex_t mut1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut2 = PTHREAD_MUTEX_INITIALIZER;

void do_one_thing() {
  pthread_mutex_lock(&mut1); // заблокировали ресурс
  sourceA++;
  sleep(1); // фора для блокировки другим потоком ресурса B
  pthread_mutex_lock(&mut2); // поток пытается заблокировать ресурс B и работать с ним
  sourceB++;                 // в этом месте ресурс будет заблокирован другим потоком
  
  pthread_mutex_unlock(&mut1);
  pthread_mutex_unlock(&mut2);
}

void do_another_thing() {
  pthread_mutex_lock(&mut2); // заблокирован ресурс B
  sourceB++;
  sleep(1); // фора для блокировки другим потоком ресурса A
  pthread_mutex_lock(&mut1); // поток пытается заблокировать ресурс A и работать с ним
  sourceA++;                 // в этом месте ресурс A будет уже заблокирован предыдущим потоком
 
  pthread_mutex_unlock(&mut2);
  pthread_mutex_unlock(&mut1);
}

int main() {

  // создание потоков для выполнения функций
  pthread_t thread1, thread2;
  if (pthread_create(&thread1, // указатель на поток
                     NULL,     // атрибуты потока
                     (void *)do_one_thing,  // указатель на функцию
			         NULL) != 0)  // аргументы потока
  {
    perror("pthread_create");
    exit(1);
  }

  if (pthread_create(&thread2, 
                     NULL, 
                     (void *)do_another_thing,
                     NULL) != 0) 
  {
    perror("pthread_create");
    exit(1);
  }

  // ожидание потоков

  if (pthread_join(thread1, NULL) != 0) {
    perror("pthread_join");
    exit(1);
  }

  if (pthread_join(thread2, NULL) != 0) {
    perror("pthread_join");
    exit(1);
  }
  printf("/n %d",sourceA);
  printf("/n %d",sourceB);
  return 0;
}