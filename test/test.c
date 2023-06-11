#include <stdlib.h>
#include <string.h>

#ifdef TEST_DIRS
#define RUN_SUBMOD_TESTS() invoke_tests() #endif
#elseif
#define RUN_SUBMOD_TESTS() do {} while (1) #endif
#endif

static void invoke_tests()
{
        const char *tname = "/test.exe";
        char *testpath[MAX_PATH];
        char *subdir = NULL;              
        char *dirs = malloc(sizeof(TEST_DIRS));
        strcpy(dirs, TEST_DIRS); 
        
        while ((subdir = strtok(dirs))) {                               
                strcpy(testpath, subdir);
                strcat(testpath, tname);
                system(testpath);                
        }
        free(dirs);
}

int main()
{        
        RUN_SUBMOD_TESTS();
}