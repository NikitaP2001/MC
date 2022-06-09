#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <string>
#include "dbg_time.hpp"
#include "lexer.hpp"

int main(int argc, char *argv[])
{
    if (argc < 2)
        std::cout << "not input files found";

    std::fstream fin(argv[1] , std::ios::in);
    std::vector<std::vector<char>> fcontent;
    std::string fline;
    while (std::getline(fin, fline)) {
        std::vector<char> lcontent(fline.begin(), fline.end());
        fcontent.push_back(lcontent);
    }

    std::vector<lexer::token> tokens; 
    lexer::get_tokens(fcontent, tokens);


    return 0;
}