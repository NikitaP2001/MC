#include <exception>
#include "file.hpp"
#include "main.hpp"

file::file(std::string fpath, bool readonly = true)
: path(fpath)
{
    DWORD dwDesiredAccess = (readonly) ? GENERIC_READ
	    : GENERIC_READ | GENERIC_WRITE;
    hFile = CreateFile(path.c_str(), dwDesiredAccess, 0,
	NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
	std::string eMsg("fail: open file "); 
	eMsg.insert(eMsg.end(), fpath.begin(), fpath.end());
	throw std::runtime_error(eMsg);
    }
}

file::~file()
{
    CloseHandle(hFile);
}

mm_file::mm_file(std::string fpath)
: file(fpath)
{
    HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hFileMapping == NULL) {
	std::string eMsg("fail: create file mapping "); 
	eMsg.insert(eMsg.end(), fpath.begin(), fpath.end());
	throw std::runtime_error(eMsg);
    }

}

mm_file::~mm_file()
{
    CloseHandle(hFileMapping);
}
