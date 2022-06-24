#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <iterator>
#include <windows.h>

#include "misc.hpp"
#include "main.hpp"
#include "file.hpp"
#include "unit.hpp"
#include "lexer.hpp"


int main(int argc, char *argv[])
{

    if (argc < 2) {
        std::cout << "not input files found";
        exit(1);
    }

    mm_file fin(argv[1], true);
    snippet sn;
    uint64_t cb = fin.read(sn);

    for (auto &ln : sn.content) {
        for (const char ch : ln)
            std::cout << ch;
    }

   
    return 0;
}
