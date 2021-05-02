#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int pid = fork();
    if (pid == 0)
    {
        execvp("sequential_min_max", argv); // vp - имя программы полностью + ищет файл используя переменную среды path
    }
    return 0;
}