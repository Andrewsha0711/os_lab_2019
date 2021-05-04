#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {

 pid_t child_pid;
 child_pid = fork(); // создали дочерний процесс
 if (child_pid > 0) 
 {
 sleep(60); // если родительский - ждем
 } 
 else 
 {
  exit(0); // если дочерний - завершаем процесс
 }
 return 0;
 // дочерний процесс не передал код завершения потомку
 // дочерний процесс не удаляется функцией wait() у родителя 
}