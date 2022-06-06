#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include "dbg_time.hpp"

int main(int argc, char *argv[])
{
    if (argc < 2)
        std::cout << "not input files found";

    std::fstream fin(argv[1] , std::ios::in);
    std::vector<char> fcontent((std::istream_iterator<char>(fin)),
    (std::istream_iterator<char>()));

    return 0;
}