#include <string>
#include <vector>
#include <list>

#include <windows.h>

class file {

    void *pmem = NULL;
    std::string path;
    HANDLE hFile == NULL;

public:

    file(std::string fpath);

    virtual void read(snippet &content) const = 0;
    virtual void flush() = 0;   // write file content to disk

    virtual ~file();

};

class mem_file {

};

class disk_file : file {

    HANDLE hFIleMapping;

    disk_file(std::string fpath);

    virtual ~disk_file();

}

/* represents some part of test file */
class snippet {
    file *f_ld_from;    // associated file
    long line;  // line in starts in file

    std::list<std::vector<char>> content;
};