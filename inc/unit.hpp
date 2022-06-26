#include <vector>
#include <filesystem>
#include "file.hpp"

namespace fs = std::filesystem;

class unit {

    // opened source files
    std::vector<file *> files;

public:

    unit(fs::path file_start, std::vector<fs::path> inc_dirs);

};