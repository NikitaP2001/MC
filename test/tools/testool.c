#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <mc.h>

#include "testool.h"

size_t cont_len(char *content)
{
        size_t length = strlen(content);
        if (content[length - 1] != '\n')
                length += 1;
        return length;
}

_Bool wordcmp(char *str1, char *str2)
{
        char c1 = '\0', c2 = '\0';
        _Bool is_eq = true;
        do {
                while (is_white_space(c1 = *str1++));
                while (is_white_space(c2 = *str2++));
                if (c1 != c2) {
                        is_eq = false;
                        break;
                }
        } while (c1 != '\0' && c2 != '\0');
        return is_eq;
}

_Bool write_file(const char *file_name, const char *content, size_t length)
{
        _Bool result = false;
        FILE *fp = fopen(file_name, "w");
        if (fp != NULL) {
                fwrite(content, sizeof(char), length, fp);

                result = ferror(fp) == 0;
                fclose(fp);
        }
        return result; 
}


void invoke_tests()
{
        static const char *tname = "/test.exe";        
        char *subdir = NULL;              
        char *dirs = malloc(sizeof(TEST_DIRS));
        strcpy(dirs, TEST_DIRS); 
        
        subdir = strtok(dirs, " ");
        if (subdir != NULL) {                
                do {         
                        char *testpath = malloc(strlen(subdir)
                        + sizeof(tname));
                        strcpy(testpath, subdir);
                        strcat(testpath, tname);
                        system(testpath);                
                        free(testpath);
                } while ((subdir = strtok(NULL, " ")));
        }
        
        free(dirs);
}