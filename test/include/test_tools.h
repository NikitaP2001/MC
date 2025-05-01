#include <stdio.h>

#define TFILE_NAME "file.cc"

static inline 
_Bool 
write_file(const char *file_name, const char *content, size_t length)
{
        _Bool result = false; /* This 'result' is local and okay */
        FILE *fp = fopen(file_name, "wb");
        if (fp != NULL) {
                fwrite(content, sizeof(char), length, fp);

                result = ferror(fp) == 0;
                fclose(fp);
        }
        return result; 
}
