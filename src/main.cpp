#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <string>
#include <cstring>
#include "dbg_time.hpp"
#include "lexer.hpp"
#include "unit.hpp"

#include <windows.h>

class task;

int main(int argc, char *argv[])
{

    if (argc < 2) {
        std::cout << "not input files found";
        exit(1);
    }

    char *fn = argv[1];
    unsigned long long t0 = read_tsc();
    std::fstream fin(argv[1], std::ios::in);
    std::vector<std::vector<char>> fcontent;
    std::string fline;

    while (std::getline(fin, fline)) {
        std::vector<char> lcontent(fline.begin(), fline.end());
        fcontent.push_back(lcontent);
    }

    std::cout << "rd time: " << read_tsc() - t0 << std::endl;

    std::vector<lexer::token> tokens; 
    lexer::get_tokens(fcontent, tokens);

    HANDLE hFin = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL, NULL);

    if (!CreateFileMapping(hFin, NULL, PAGE_READONLY, 0, 0, NULL))
        std::cerr << "Create mapping failed" << std::endl;

/*
    for (auto &ln : fcontent) {
        for (auto ch : ln)
            std::cout << ch;
        std::cout << '\n';
    } */

    return 0;
}
