#include <stdio.h>

#include <tools.h>

int get_file_size(FILE *fp)
{
        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        return size;
}