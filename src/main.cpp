#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <iterator>
#include <windows.h>

#include "main.hpp"
#include "lexer.hpp"
#include "unit.hpp"
#include "misc.hpp"
#include "file.hpp"


int main(int argc, char *argv[])
{

    if (argc < 2) {
        std::cout << "not input files found";
        exit(1);
    }

    int fsize = 10000;
    int wr_cnt = 1000;
    fs::path ftest = "testfile";
    if (fs::exists(ftest))
        fs::remove(ftest);

    // generate random file content
    srand(time(NULL));
    char *fdat = new char[fsize];
    char *rdbuf = new char[fsize];
    for (int i = 0; i < fsize; i++)
        fdat[i] = static_cast<char>(rand() % 0xFF);

    std::ofstream stest(ftest.generic_string(), std::ios::binary);
    stest.write(fdat, fsize); 
    stest.close();

    {
        mm_file fout(ftest.generic_string(), false);

        for (int ntest; ntest < wr_cnt; ntest++) {
            int wrpos = rand() % fsize;
            int wrlen = rand() % (fsize - wrpos - 1) + 1;
            for (int i = 0; i < wrlen; i++) {
                char brnd = static_cast<char>(rand() % 0xFF);
                fdat[wrpos + i] = brnd;
            }
            fout.write(&fdat[wrpos], wrpos, 10000);
        }
    }

    delete[] rdbuf;
    delete[] fdat;

    return 0;
}
