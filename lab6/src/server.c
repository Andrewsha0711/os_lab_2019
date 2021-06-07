#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "pthread.h"
#include "sources.h"

struct FactorialArgs {
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

uint64_t Factorial(const struct FactorialArgs *args) {
  uint64_t ans = 1;
  for (uint64_t i = (*args).begin; i < (*args).end; i++)
  {
      ans = MultModulo(ans, i, (*args).mod); // перемножение по модулю
  }
  printf("new thread worked out on this server, params: left_point %llu, right_point %llu, mod %llu - result %llu\n", (*args).begin, (*args).end-1, (*args).mod, ans);
  return ans;
}

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  return (void *)(uint64_t *)Factorial(fargs);
}

int main(int argc, char **argv) {
  int tnum = -1;
  int port = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"tnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        port = atoi(optarg);
        if (port <= 0)
        {
            printf("port is a positive number\n");
            return 1;
        }
        break;
      case 1:
        tnum = atoi(optarg);
        if (tnum <= 0)
        {
            printf("tnum is a positive number\n");
            return 1;
        }
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Unknown argument\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (port == -1 || tnum == -1) {
    fprintf(stderr, "Using: %s --port PORT_NUMBER --tnum 4\n", argv[0]);
    return 1;
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0); // создание сокета
  if (server_fd < 0) {
    fprintf(stderr, "Can not create server socket!");
    return 1;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port); // преобразование в сетевой порядок битов
  server.sin_addr.s_addr = htonl(INADDR_ANY);  // преобразование в сетевой порядок битов

  int opt_val = 1;
  // установка атрибутов
  setsockopt(server_fd,        // дескриптор
             SOL_SOCKET,       // уровень протокола
             SO_REUSEADDR,     // имя атрибута
             &opt_val,         // значение атрибута
             sizeof(opt_val)   // длина атрибута
             );

  /* Параметр SO_REUSEADDR позволяет прослушивающему серверу запуститься и с помощью функции bind связаться 
  со своим заранее известным портом */

  int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server)); // привязывает к сокету адрес типа sockaddr
  if (err < 0) {
    fprintf(stderr, "Can not bind to socket!");
    return 1;
  }

  err = listen(server_fd, 128); // переводит сокет в режим LISTEN, второй аргумент - максимальное число соединений
  if (err < 0) {
    fprintf(stderr, "Could not listen on socket\n");
    return 1;
  }

  printf("Server listening at %d\n", port);

  while (true) {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client); // длина структуры адреса
    int client_fd = accept(server_fd,                      // функция для принятия связи на сокет
                                                           // server_fd - сокет дескриптор для принятия связи от клиента
                           (struct sockaddr *)&client,     // указатель на адрес клиента
                           &client_len);                   // длина структуры адреса

    if (client_fd < 0) {
      fprintf(stderr, "Could not establish new connection\n");
      continue;
    }

    while (true) {
      unsigned int buffer_size = sizeof(uint64_t) * 3;
      char from_client[buffer_size];
      int read = recv(client_fd, from_client, buffer_size, 0); // получение данных из сокета при установленном соединени

      if (!read)
        break;
      if (read < 0) {
        fprintf(stderr, "Client read failed\n");
        break;
      }
      if (read < buffer_size) {
        fprintf(stderr, "Client send wrong data format\n");
        break;
      }

      pthread_t threads[tnum];

      uint64_t begin = 0;
      uint64_t end = 0;
      uint64_t mod = 0;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

      fprintf(stdout, "Receive: %llu %llu %llu\n", begin, end, mod);

      struct FactorialArgs args[tnum];
      for (uint32_t i = 0; i < tnum; i++) {
          // заполняем наборы агрументов
        args[i].begin = begin + (end-begin+1)/tnum*i;
        args[i].end = i==tnum-1?end+1:(begin + (end-begin+1)/tnum*(i+1));
        args[i].mod = mod;
          // параллелим по потокам
        if (pthread_create(&threads[i], NULL, ThreadFactorial,
                           (void *)&args[i])) {
          printf("Error: pthread_create failed!\n");
          return 1;
        }
      }

      uint64_t total = 1;
      for (uint32_t i = 0; i < tnum; i++) {
        uint64_t result = 0;
        pthread_join(threads[i], (void **)&result); // собираем результаты работы потоков
        total = MultModulo(total, result, mod);     // объединяем с общим результатом
      }

      printf("Total: %d\n", total);

      char buffer[sizeof(total)];
      memcpy(buffer, &total, sizeof(total));
      // отправление буфера с данными на сокет
      err = send(client_fd,                   // сокет - дескриптор, в который записываются данные
                 buffer,                      // адрес буфера с данными
                 sizeof(total),               // длина буфера с данными
                 0                            /* комбинация флагов, отвечающих за тип передачи данных      
                                                 в данном случае по умолчанию */                                         
                 );
      if (err < 0) {
        fprintf(stderr, "Can't send data to client\n");
        break;
      }
    }

    shutdown(client_fd, SHUT_RDWR); // принудительное закрытие связи на сокет
    close(client_fd);
  }

  return 0;
}