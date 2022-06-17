#include "file.hpp"

file::file(std::string path)
: path(fpath)
{
    HANDLE hFile = CreateFile(path.c_str(), GENERIC_READ, 0,
    NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        ERR2("create file failed", );
        exit(1);
    }
}

file::~file()
{

    CloseHandle(hFile);
}

disk_file::disk_file(std::string fpath)
: file(fpath)
{
    

}

disk_file::~disk_file()
{

}