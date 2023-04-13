#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void funct(const char *param)
{
        printf("%s", param);
}

int main()
{
     char arr[10];
     strncpy(arr, "hello gpt", sizeof("hello gpt"));
     funct(arr);
     exit(0);
}