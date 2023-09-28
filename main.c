#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <parser.h>

int main()
{
        struct parser ps = {0};
        parser_init(&ps, psym_constant);
        puts("It`s not time yet");
}