
/* Program to display address information about the process */ // программа для отображения адресной информации о процессе
/* Adapted from Gray, J., program 1.4 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* Below is a macro definition */ // определение макроса
#define SHW_ADR(ID, I) (printf("ID %s \t is at virtual address: %8X\n", ID, &I)) // ID (имя функции) + адрес

extern int etext, edata, end; /* Global variables for process
                                 memory */ // глобальные переменные для памяти процесса

char *cptr = "This message is output by the function showit()\n"; /* Static */ // сообщение для вывода функцией showit
char buffer1[25];
int showit(); /* Function prototype */

main() 
{
  int i = 0; /* Automatic variable */ // автоматическая переменная

  /* Printing addressing information */ // печать адресной информации процесса
  printf("\nAddress etext: %8X \n", &etext); //первый адрес за концом сегмента кода программы
  printf("Address edata: %8X \n", &edata); // первый адрес за концом сегмента инициализированных данных
  printf("Address end  : %8X \n", &end); //первый адрес за концом сегмента неинициализированных данных (сегмента BSS)
  
  SHW_ADR("main", main);
  SHW_ADR("showit", showit);
  SHW_ADR("cptr", cptr);
  SHW_ADR("buffer1", buffer1);
  SHW_ADR("i", i);
  strcpy(buffer1, "A demonstration\n");   /* Library function */
  write(1, buffer1, strlen(buffer1) + 1); /* System call */
  showit(cptr);

} /* end of main function */

/* A function follows */
int showit(p) char *p;
{
  char *buffer2;
  SHW_ADR("buffer2", buffer2);
  if ((buffer2 = (char *)malloc((unsigned)(strlen(p) + 1))) != NULL) 
  {
    printf("Alocated memory at %X\n", buffer2);
    strcpy(buffer2, p);    /* copy the string */
    printf("%s", buffer2); /* Didplay the string */
    free(buffer2);         /* Release location */
  } else {
    printf("Allocation error\n");
    exit(1);
  }
}