#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdbool.h>
#include <getopt.h>

//#define BUFSIZE 100	
#define SADDR struct sockaddr
#define SIZE sizeof(struct sockaddr_in)
	
int main(int argc, char *argv[]) {
	int buffsize = -1;
	int server_port = -1;
	char* address = NULL;
	while (true) {
		int current_optind = optind ? optind : 1;
	
	    static struct option options[] = {{"buffsize", required_argument, 0, 0},
	                                      {"server_port", required_argument, 0, 0},
	                                      {"address", required_argument, 0, 0},
	                                      {0, 0, 0, 0}};
	
	    int option_index = 0;
	    int c = getopt_long(argc, argv, "f", options, &option_index);
	
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
	            if (server_port <= 0) {
	                printf("server port is a positive number\n");
	                return 1;
	              }
	            break;
	            case 2:
	            address = optarg;
	            break;
	        }
	
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

	  if (buffsize == -1 || server_port == -1) {
	    printf("Usage: %s --buffsize \"buffer_size\" --server_port \"server_port\" --address \"address\n",
	           argv[0]);
	    return 1;
	  }
	
	  int fd;
	  int nread;
	  char buf[buffsize];
	  struct sockaddr_in servaddr;
	  if (argc < 3) {
	    printf("Too few arguments \n");
	    exit(1);
	  }
	
	  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    perror("socket creating");
	    exit(1);
	  }
	
	  memset(&servaddr, 0, SIZE);
	  servaddr.sin_family = AF_INET;
	
	  if (inet_pton(AF_INET, address, &servaddr.sin_addr) <= 0) // преобразует строку в структуру с сетевым адресом
      {
	    perror("bad address");
	    exit(1);
	  }

	  servaddr.sin_port = htons(server_port);
	
	  if (connect(fd, (SADDR *)&servaddr, SIZE) < 0) {
          perror("connect");
          exit(1);
          }
	
	  write(1, "Input message to send\n", 22);
	  while ((nread = read(0, buf, buffsize)) > 0) {
	    if (write(fd, buf, nread) < 0) {
	      perror("write");
	      exit(1);
	    }
	  }
	
	  close(fd);
	  exit(0);
	}
