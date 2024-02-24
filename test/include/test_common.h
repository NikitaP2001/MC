#include <stdio.h>

#define LEN_OF(str) (sizeof(str) - 1)

static inline 
_Bool 
write_file(const char *file_name, const char *content, size_t length)
{
        _Bool result = false;
        FILE *fp = fopen(file_name, "wb");
        if (fp != NULL) {
                fwrite(content, sizeof(char), length, fp);

                result = ferror(fp) == 0;
                fclose(fp);
        }
        return result; 
}