#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <iterator>
#include <windows.h>

#include "misc.hpp"
#include "main.hpp"
#include "unit.hpp"
#include "preprocessor.hpp"


int main(int argc, char *argv[])
{

    if (argc < 2) {
        std::cout << "not input files found";
        exit(1);
    }

    mm_file fin(argv[1], true);    
    snippet sn;
    fin.read(sn);
    purge_comments(sn);

    for (auto *ln : sn.lines) {
        auto *pln = ln->get_ptr();
        for (int i = 0; i < ln->size(); i++)
            std::cout << pln[i];
    }


}
