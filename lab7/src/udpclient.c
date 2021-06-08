#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdbool.h>
#include <getopt.h>

// #define SERV_PORT 20001
// #define BUFSIZE 1024
#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char **argv) {
  int buffsize = -1;
  int server_port = -1;
  char* address = "\0";
  while (true) {
      int current_optind = optind ? optind : 1;
      static struct option options[] = {{"buffsize", required_argument, 0, 0},
	                                      {"server_port", required_argument, 0, 0},
	                                      {"address", required_argument, 0, 0},
	                                      {0, 0, 0, 0}};
	  int option_index = 0;
      int c = getopt_long(argc, argv, "",options, &option_index);
      if (c == -1) break;
      
      switch (c) {
          case 0:
          switch (option_index) {
              case 0:
              buffsize = atoi(optarg);
              if (buffsize <= 0) {
                  printf("buffer size must be a positive number\n");
	              return 1;
                  }
              break;
	
	          case 1:
              server_port = atoi(optarg);
              if (server_port <= 0) {
                  printf("server port is a positive number\n");
                  return 1;
                  }
	          break;
              case 2:
              address = optarg;
              if (address == "\0"){
                  printf("address is not correct\n");
                  return 1;
                  }

              break;
	        }
	
	      case '?':
	        break;
	
	      default:
	        printf("getopt returned character code 0%o?\n", c);
	    }
	
	  }
	  if (buffsize == -1 || server_port == -1 || address == "\0") {
	    printf("Usage: %s --buffsize \"buffer_size\" --server_port \"server_port\" --address \"address\"\n",
	           argv[0]);
	    return 1;
	  }
  int sockfd, n;
  char sendline[buffsize], recvline[buffsize + 1];
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(server_port);

  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 0) { // создание структуры с сетевым адресом
    perror("inet_pton problem");
    exit(1);
  }
  if ((sockfd = socket(AF_INET,
                       SOCK_DGRAM, /* в отличии от предыдущего случая (SOCK_STREAM), 
                                   обеспечивает передачу данных в виде отдельных сообщений (датаграмм), 
                                   без предварительной установки соединения */
                       0)
                       ) < 0) {  // неявное создание сокета
    perror("socket problem");
    exit(1);
  }

  write(1, "Enter string\n", 13); 

  while ((n = read(0, sendline, buffsize)) > 0) { // неявное считывание с терминала в sendline
    if (sendto(  // неявная отправка на сокет, в отличие от send может отправлять без установки соединения (в любое время)
               sockfd,                 // сокет
               sendline,               // буфер
               n,                      // длина
               0,                      // флаги
               (SADDR *)&servaddr,     // адрес назначения, так как не установлено соединение
               SLEN)                   // длина адреса
               == -1) {
      perror("sendto problem");
      exit(1);
    }

    if (recvfrom(sockfd, recvline, buffsize, 0, NULL, NULL) == -1) {
      perror("recvfrom problem");
      exit(1);
    }

    printf("REPLY FROM SERVER= %s\n", recvline);
  }
  close(sockfd);
}
