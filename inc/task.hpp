#include <map>
#include <set>
#include <vector>
#include <string>
#include <exception>
#include <filesystem>
namespace fs = std::filesystem;

enum command_type {
    NO_COMMAND,
    FILE_OUT,   // -o
    NOT_LINK,   // -c
    INC_DIRS,   // -I
    LIB_DIRS,   // -L
};

extern std::map<std::string, command_type> str_com_map;

class task {
    std::set<command_type> commands;

protected:

    fs::path fout;
    std::vector<fs::path> files;
    std::vector<fs::path> inc_dirs;
    std::vector<fs::path> lib_dirs;

public:
    #define EX_MSG_SIZE 100
    #define EX_TYPE "Argparse error: "
    task(int argc, const char *argv[]);
    

};
