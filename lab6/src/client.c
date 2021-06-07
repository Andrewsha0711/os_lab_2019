#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <pthread.h>
#include "sources.h"

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

struct Server {
  char ip[255];
  uint64_t port;
};

typedef struct
{
  struct sockaddr_in server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
} ServArgs;

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL; /* адрес следующего символа в строке string, сразу после предыдущего найденного числа
                       в данном случае не используется поэтому = NULL */
  int base = 10;    /* основание системы счисления */
  unsigned long long i = strtoull(str, &end, base); // преобразование строки в значение типа unsigned long int
  if (errno == ERANGE) // обработка ошибки error range (результат слишком велик)
  {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

uint64_t WorkWithServer(ServArgs *args)
{
    int sck = socket(AF_INET, SOCK_STREAM, 0); /* создание сокета, атрибуты:
                                                  домен, тип, протокол
                                                  AF_INET задает домен Internet 
                                                  SOCK_STREAM - передача потока данных с предварительной 
                                                  установкой соединения
                                                  Протокол определяется по доменту и типу сокета, можно передать в
                                                  протокол 0, что соответствует протоколу по умолчанию
                                                  */
    if (sck < 0) {
      fprintf(stderr, "Socket creation failed!\n");
      exit(1);
    }

    if (connect(sck, (struct sockaddr *)&args->server, sizeof(args->server)) < 0) 
    /* Неявное связывание сокета с адресом в выранном домене*/
    {
      fprintf(stderr, "Connection failed\n");
      exit(1);
    }
    
    char task[sizeof(uint64_t) * 3];
    memcpy(task, &args->begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &args->end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &args->mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
      fprintf(stderr, "Send failed\n");
      exit(1);
    }

    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
      fprintf(stderr, "Recieve failed\n");
      exit(1);
    }
    uint64_t answer = 0;
    memcpy(&answer, response, sizeof(uint64_t));
    close(sck);
    return answer;
}

int main(int argc, char **argv)
{
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'}; // TODO: explain why 255

  while (true)
  {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c)
    {
      case 0:
      {
        switch (option_index)
        {
          case 0:
            ConvertStringToUI64(optarg, &k);
            if (k <= 0)
            {
              printf("k is a positive number\n");
              return 1;
            }
            break;
          case 1:
            ConvertStringToUI64(optarg, &mod);
            if (mod <= 0)
            {
              printf("mod is a positive number\n");
              return 1;
            }
            break;
          case 2:
            memcpy(servers, optarg, strlen(optarg));
            if (strlen(servers) == sizeof('\0'))
            {
              printf("servers is a path to file with ports\n");
              return 1;
            }
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
      }
      break;

      case '?':
        printf("Arguments error\n");
        break;
      default:
        fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers))
  {
  fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
          argv[0]);
  return 1;
  }
  
  unsigned int servers_num = 0;
  struct Server *to;
  FILE* f = fopen(servers, "r");
  if(!f)
	{
	  printf("server file error!\n");
	  return 1;
	}
  char *str = malloc(256*sizeof(char));
  char *istr = malloc(256*sizeof(char));
  while (fgets(str, // указатель на массив, куда сохраняются символы
               255, // максимальное количество символов для чтения, включая нулевой символ
               f    // файловый поток
               ) != NULL) // посимвольное считывание
  {
    servers_num++;
    if (servers_num == 1)
    {
      to = malloc(sizeof(struct Server) * servers_num);
    }
    else
    {
      to = realloc(to, sizeof(struct Server) * servers_num);
    }
    istr = strtok(str, ":");
    strcpy(to[servers_num-1].ip, istr);
    istr = strtok(NULL, "\n");
    ConvertStringToUI64(istr, &to[servers_num-1].port);
  }
  fclose(f);

  uint64_t part = k/servers_num;

  // TODO: work continiously, rewrite to make parallel
  uint64_t begin;
  uint64_t end;
  uint64_t answer = 1;
  
  ServArgs args[servers_num]; // наборы аргументов
  pthread_t threads[servers_num];  // потоки

  for (int i = 0; i < servers_num; i++)
  {
    struct hostent *hostname = gethostbyname(to[i].ip); /* аргумент - имя хоста/домена.
                                                        преобразует имя хоста/домена в формат адреса IPv4 */
    if (hostname == NULL)
    {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      exit(1);
    }

    args[i].server.sin_family = AF_INET;
    args[i].server.sin_port = htons((uint64_t)to[i].port); // преобразование в сетевой порядок расположения байтов
    args[i].server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    args[i].begin = 1 + i*part;
    if (i == (servers_num - 1))
    {
      args[i].end = k;
    }
    else
    {
      args[i].end = (i+1)*part;
    }
    args[i].mod = mod;

    pthread_create(&threads[i], NULL, WorkWithServer, (void *)&args[i]);
  }

  for (uint64_t i = 0; i <servers_num; i++) // собираем результаты с потоков
  {
    pthread_mutex_lock(&mut);
    int between_answer=0;
    pthread_join(threads[i],(void**)&between_answer);
    answer =  MultModulo(answer, between_answer, mod);
    pthread_mutex_unlock(&mut);
  }

  printf("answer: %llu\n", answer);

  free(to);

  return 0;
}