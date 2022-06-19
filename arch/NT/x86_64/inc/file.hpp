#include <string>
#include <vector>
#include <list>

#include <windows.h>


struct snippet;

class file {
protected:

    std::string path;
    HANDLE hFile = INVALID_HANDLE_VALUE;

public:

    file(std::string fpath, bool readonly);

    virtual void read(snippet &content) const = 0;
    virtual void write(void *src, void *f_offset, int size) = 0;   // write file content to disk

    virtual ~file();

};

#define MMFILE_SIZE_MAX 4294967296

/* memory mapped file object
 */
class mm_file : file {
private:
    LPVOID pView = NULL;	// file content in memory
    DOWRD FileSize; 
    
    HANDLE hFileMapping;

public:
    virtual void read(snippet &content) const = 0;
    virtual void write(snippet &content) const = 0;

    mm_file(std::string fpath);

    ~mm_file();

};

/* represents some part of text file */
struct snippet {
    file *f_ld_from;    // associated file
    long line;  // line it starts in file

    std::list<std::vector<char>> content;
};


