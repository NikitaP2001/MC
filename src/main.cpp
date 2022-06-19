#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <iterator>
#include "main.hpp"
#include "lexer.hpp"
#include "unit.hpp"
#include "dbg_time.hpp"

#include <windows.h>


class task;

int main(int argc, char *argv[])
{

    if (argc < 2) {
        std::cout << "not input files found";
        exit(1);
    }

    char *fn = argv[1];
    std::list<std::vector<char>> fcontent;

    unsigned long long t0 = read_tsc();

    HANDLE hFin = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFin == INVALID_HANDLE_VALUE) {
        ERR2("create file failed");
        exit(1);
    }

    HANDLE hfm = CreateFileMapping(hFin, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hfm == NULL) {
        ERR2("create mapping failed");
        exit(1);
    }

    char *pdata = (char *)MapViewOfFile(hfm, FILE_MAP_READ, 0, 0, 0);
    if (pdata == NULL)
        ERR2("Map view failed");

    MEMORY_BASIC_INFORMATION hfmminfo;
    if (VirtualQuery(pdata, &hfmminfo, sizeof(hfmminfo)) == 0)
        ERR2("VirtualQuery failed");

    for (char *p = pdata, *prevln = pdata;
    p - pdata < hfmminfo.RegionSize; p++) {
        if (*p == '\n') {
            std::vector<char> vline(prevln, p);
            vline.pop_back();
            fcontent.push_back(vline);
            prevln = p;
        }
    }

    std::cout << "rd time: " << read_tsc() - t0 << std::endl;

    if (UnmapViewOfFile(pdata) == 0)
        ERR2("Unmap view failed");

    CloseHandle(hfm);
    CloseHandle(hFin);

/*
    for (auto &ln : fcontent) {
        for (auto ch : ln)
            std::cout << ch;
        std::cout << '\n';
    } */

    return 0;
}
