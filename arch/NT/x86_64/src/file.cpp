#include <stdexcept>
#include <algorithm>
#include <stdlib.h>
#include "file.hpp"
#include "main.hpp"

file::file(std::string fpath, bool readonly)
: path(fpath), _readonly(readonly)
{
    DWORD dwCreationDisposition;
    DWORD dwDesiredAccess;

    // open file for read / readwrite
    if (readonly) {
        dwDesiredAccess = GENERIC_READ;
        dwCreationDisposition = OPEN_EXISTING;
    } else {
        dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        dwCreationDisposition = OPEN_ALWAYS;
    }
    hFile = CreateFile(path.c_str(), dwDesiredAccess, 0,
	NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        ERR2("CreateFile error");
        std::string eMsg = "fail: open file " + fpath;
        throw std::runtime_error(eMsg);
    }

    // read file size to class fileld
    DWORD fszLow;
    DWORD fszHigh;
    fszLow = GetFileSize(hFile, &fszHigh);
    if (fszLow == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
        ERR2("GetFileSize error");
        std::string eMsg = "fail: get file info " + fpath;
        throw std::runtime_error(eMsg);
    }
    FileSize = ((uint64_t)fszHigh << 0x20) + fszLow;
    if (readonly && FileSize == 0) {
        std::string eMsg = fpath + ": file not found";
        throw std::runtime_error(eMsg);
    }
}

std::string file::get_path()
{
    return path;
}

file::~file()
{
    // correct end of file
    if (!_readonly) {
        LONG MvDistLow = FileSize & 0xFFFFFFFF;
        LONG MvDistHigh = FileSize >> 0x20;
        DWORD err = SetFilePointer(hFile, MvDistLow, &MvDistHigh, FILE_BEGIN);
        if (err == INVALID_SET_FILE_POINTER)
            ERR2("Set end of " << path << " failed");
        if (!SetEndOfFile(hFile))
            ERR2("SetEndOfFile failed");
    }
    CloseHandle(hFile);
}

mm_file::mm_file(std::string fpath, bool readonly)
: file(fpath, readonly)
{
    DWORD flProtect;
    DWORD MaxSzHigh;
    DWORD MaxSzLow;

    // limit supported mmfile size
    if (FileSize > MMFILE_SIZE_MAX) {
        ERR(fpath << " have size over MMFILE_SIZE_MAX");
        std::string eMsg = fpath + ": file size not supported";
        throw std::runtime_error(eMsg);
    }

    // couse inpossible map empty file
    if (FileSize == 0) {
        SetFilePointer(hFile, 1, NULL, FILE_BEGIN);
        int res = SetEndOfFile(hFile);
        FileSize = 1;
    }

    if (readonly) {
        flProtect = PAGE_READONLY;
        // size of mapping == file size
        MaxSzHigh = FileSize >> 0x20;
        MaxSzLow = FileSize & 0xFFFFFFFF;
    } else {
        flProtect = PAGE_READWRITE;    

        // size of mapping == max supported;
        MaxSzHigh = MMFILE_SIZE_MAX >> 0x20;
        MaxSzLow = MMFILE_SIZE_MAX & 0xFFFFFFFF;
    }

    hFileMapping = CreateFileMapping(hFile, NULL, flProtect,
    MaxSzHigh, MaxSzLow, NULL);
    if (hFileMapping == NULL) {
        ERR2("CreateFileMapping error");
        std::string eMsg = "fail: create file mapping " + path;
        throw std::runtime_error(eMsg);
    }

    // map view of entrire file
    DWORD dwAccess = (readonly) ? FILE_MAP_READ
    : FILE_MAP_READ | FILE_MAP_WRITE;
    pView = MapViewOfFile(hFileMapping, dwAccess, 0, 0, FileSize);
    if (pView == NULL) {
        ERR2("MapViewOfFile error");
        std::string eMsg = "fail: map view of file " + path;
        throw std::runtime_error(eMsg);
    }
}

/* read @count bytes to @dest from file, begining at
 * position @f_offset
 *  return number of bytes read */
uint64_t mm_file::read(void *dest, uint64_t f_offset, uint64_t count) const
{
    if (f_offset > FileSize)
        return 0;
    if (f_offset + count > FileSize)
        count = FileSize - f_offset;
    
    memmove(dest, static_cast<char *>(pView) + f_offset, count);
    
    return count;
}

/* put all file content to snippet structure,
 *  return number of bytes read */
uint64_t mm_file::read(snippet &content) const
{
    uint64_t fpos = 0;
    char *pbeg = static_cast<char *>(pView);

    // clear old data, if present
    content.clear(); 

    int line_num = 0;
    while (fpos < FileSize) {
        uint64_t oldpos = fpos;
        for (char c = *(pbeg+fpos++); fpos < FileSize && c != '\n'; fpos++)
            c = *(pbeg+fpos);
        line *nln = new line(pbeg + oldpos,
        fpos - oldpos, line_num);
        nln->f_ln_from = this;
        content.lines.push_back(new line(pbeg + oldpos,
        fpos - oldpos, line_num));

        line_num += 1;
    }
    return fpos;
}

/* write @count bytes to file view by @f_offset offset
 *  return number of bytes written */
uint64_t mm_file::write(void *src, uint64_t f_offset, uint64_t count)
{
    if (_readonly) {
        ERR("Attempt to write rdonly file");
        std::string eMsg = "Unable to write to rdonly file " + path;
        throw std::logic_error(eMsg);
    }

    if (f_offset + count > FileSize)
        resize(f_offset + count);

    memmove((char *)pView + f_offset, src, count);

    return count;
}

void mm_file::resize(uint64_t new_size)
{
    if (_readonly) {
        ERR("Attempt to resize rdonly file");
        std::string eMsg = "Unable to resize rdonly file " + path;
        throw std::logic_error(eMsg);
    }

    if (new_size > MMFILE_SIZE_MAX) {
        ERR("new_size over MMFILE_SIZE_MAX");
        std::string eMsg = path + ": new size invalid";
        throw std::invalid_argument(eMsg);
    }

    if (UnmapViewOfFile(pView) == 0) {
        ERR2("UnmapViewOfFile error");
        std::string eMsg = "failed resize file " + path;
        throw std::runtime_error(eMsg);
    }

    FileSize = new_size;

    DWORD dwAccess = (_readonly) ? FILE_MAP_READ
    : FILE_MAP_READ | FILE_MAP_WRITE;
    pView = MapViewOfFile(hFileMapping, dwAccess, 0, 0, FileSize);
    if (pView == NULL) {
        ERR2("MapViewOfFile error");
        std::string eMsg = "fail: map view of file " + path;
        throw std::runtime_error(eMsg);
    }
}

uint64_t file::size()
{
    return FileSize;
}

mm_file::~mm_file()
{
    if (pView != NULL) {
        if (!UnmapViewOfFile(pView))
            ERR2("UnmapViewOfFile failed");
    }
    CloseHandle(hFileMapping);
}

line::line(char *pdata, int data_size, int ln_num)
: pline(pdata), line_size(data_size), line_number(ln_num)
{

}

const char *const line::get_ptr() const
{
    return pline;
}

int line::get_number() const
{
    return line_number;
}

int line::write(const void *src, int offset, int count)
{
    int new_sz = (offset + count > line_size) ? offset + count : line_size;
    resize(new_sz);
    memmove(pline + offset, src, count);

    return count;
}

int line::resize(int new_size)
{
    char *pnew = (is_changed) ? (char *)realloc(pline, new_size)
    : (char *)malloc(new_size);
    if (pnew == NULL){
        ERR2("realloc failed");
        std::string eMsg = "memory alloc error";
        throw std::runtime_error(eMsg);
    }

    if (!is_changed) {
        int sz = (line_size < new_size) ? line_size : new_size;
        memmove(pnew, pline, sz);
        is_changed = true;
    }

    // fill new bytes with spaces
    if (new_size > line_size)
        memset(pnew + line_size, ' ', new_size - line_size);

    line_size = new_size;
    pline = pnew;

    return new_size;
}

int line::insert(const void *src, int offset, int count)
{
    int oldsz = line_size;
    if (offset < line_size)
        resize(line_size + count);
    else
        resize(offset + count);

    if (offset < oldsz)    
        memmove(pline + offset + count, pline + offset, oldsz - offset);
    memmove(pline + offset, src, count);

    return count;
}

int line::size() const
{
    return line_size;
}

line::~line()
{
    if (is_changed)
        free(pline);
}
