#include "revert_string.h"
#include <string.h> 

void RevertString(char *str)
{
	char *start = str; // указатель на начало строки
    char *end = str + strlen(str) - 1; // указатель на конец строки
    
    while (start < end)  // выполнится когда дойдем до середины 
    {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }

}

