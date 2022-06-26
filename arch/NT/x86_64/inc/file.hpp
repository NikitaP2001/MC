/*
 *  Module file represents basic filesystem file
 *  and memory mapped file
 */

#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <vector>
#include <list>

#include <windows.h>


struct snippet;

class file {
protected:

    std::string path;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    uint64_t FileSize; 
    bool _readonly;

public:

    file(std::string fpath, bool readonly);

    virtual uint64_t read(snippet &content) const = 0;

    virtual uint64_t read(void *dest, uint64_t f_offset, uint64_t size) const = 0;

    virtual uint64_t write(void *src, uint64_t f_offset, uint64_t size) = 0;

    virtual void resize(uint64_t new_size) = 0;

    std::string get_path();

    uint64_t size();

    virtual ~file();

};

#define MMFILE_SIZE_MAX 4294967296

/* memory mapped file object
 */
class mm_file : file {
private:
    LPVOID pView = NULL;	// file content in memory
    
    HANDLE hFileMapping;

public:

    virtual uint64_t read(void *dest, uint64_t f_offset, uint64_t count) const;

    virtual uint64_t read(snippet &content) const;

    virtual uint64_t write(void *src, uint64_t f_offset, uint64_t count);

    virtual void resize(uint64_t new_size);

    mm_file(std::string fpath, bool readonly);

    ~mm_file();

};

struct line {
private:
    char *pline;

    int line_size;

    // line number within source file
    int line_number;

    bool is_changed = false;

    int resize(int new_size);

public:

    line(char *pdata, int data_size, int ln_num);

    /* attention - returned pointer may become 
     * invalid after line write operations */
    const char *const get_ptr() const;

    int get_number() const;

    /* note, that allocated mem out of initial line
     * and write interval will be filled with spaces */
    int write(const void *src, int offset, int count);

    int insert(const void *src, int offset, int count);

    int size() const;

    line(const line &) = delete;

    line &operator=(const line &) = delete;

    ~line();

};

/* represents some part of text file */
struct snippet {

    std::list<line *> lines;

    void clear()
    {
        for (auto *ln : lines)
            delete ln;
        lines.clear();
    }

    file *f_ld_from;    // associated file

    ~snippet()
    {
        clear();
    }
};

#endif // FILE_HPP