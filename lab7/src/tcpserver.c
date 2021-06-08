#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>

//#define SERV_PORT 10050
//#define BUFSIZE 100
#define SADDR struct sockaddr

int main(int argc, char *argv[]) {
    int server_port = -1;
    int buffsize = -1;
    
      while (1) {
        int current_optind = optind ? optind : 1;
    
        static struct option options[] = {{"buffsize", required_argument, 0, 0},
                                          {"server_port", required_argument, 0, 0},
                                          {0, 0, 0, 0}};
    
        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index); 
        
        if (c == -1) break;
    
        switch (c) {
          case 0:
            switch (option_index) {
              case 0:
                buffsize = atoi(optarg);
                if (buffsize <= 0) {
	                printf("size of buffer is a positive number\n");
	                return 1;
	              }
                break;
              case 1:
                server_port = atoi(optarg);
                if (server_port < 0) { 
                    printf("port is a positive number\n");
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
    
      if (buffsize == -1 || server_port == -1) {
        printf("Usage: %s --buffsize \"buffer_size\" --server_port \"server_port\n", argv[0]);
        return 1;
      }
      
  const size_t kSize = sizeof(struct sockaddr_in);

  int lfd, cfd;
  char buf[buffsize];
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  memset(&servaddr, 0, kSize);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(server_port);

  if (bind(lfd, (SADDR *)&servaddr, kSize) < 0) {
    perror("bind");
    exit(1);
  }

  if (listen(lfd,       // сокет для прослушивания
             5          // максимальное количество соединений
             ) < 0) {
    perror("listen");
    exit(1);
  }

  int nread; // для считывания и переписывания байтов
  while (1) {
    unsigned int clilen = kSize;

    if ((cfd = accept(lfd, (SADDR *)&cliaddr, &clilen)) < 0) { // принятие связи на сокет
      perror("accept");
      exit(1);
    }
    printf("connection established\n");

    while ((nread = read(cfd, buf, buffsize)) > 0) {  // неявное считывание buffsize байт из cfd в буфер buf
      write(1, &buf, nread);                          // переписывание nread байт из буфера buf в файловый указатель
    }
    if (nread == -1) {
      perror("read");
      exit(1);
    }
    close(cfd);
  }
}
