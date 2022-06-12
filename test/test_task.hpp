#include <gtest/gtest.h>
#include "task.hpp"

class TaskTest : task {
public:

    fs::path _fout;
    std::vector<fs::path> _files;
    std::vector<fs::path> _inc_dirs;
    std::vector<fs::path> _lib_dirs;

    TaskTest(int argc, const char *argv[]) : task(argc, argv)
    {
        _files = files;
        _inc_dirs = inc_dirs;
        _lib_dirs = _lib_dirs;
    }

};

TEST(TaskTest, FilesAndFlag) {
    const char *argv[] = {
        "test.exe", "file1", "file2",
        "-I", "incdir1", "-I", "incdir2",
        "-L", "libdir1", "-L", "libdir2",
        "-c"
    };

    ASSERT_NO_THROW(TaskTest test(12, argv));
    TaskTest test(12, argv);

    std::vector<fs::path> exp_files = { "file1", "file2" };
    std::vector<fs::path> exp_inc = { "incdir1", "incdir2" };
    std::vector<fs::path> exp_lib = { "libdir1", "libdir2" };

    for (auto it1 = test._files.begin(), it2 = exp_files.begin();
    it1 != test._files.end() && it2 != exp_files.end(); it1++, it2++) {
        EXPECT_TRUE(*it1 == *it2);
    }

    for (auto it1 = test._inc_dirs.begin(), it2 = exp_inc.begin();
    it1 != test._inc_dirs.end() && it2 != exp_inc.end(); it1++, it2++) {
        EXPECT_TRUE(*it1 == *it2);
    }

    for (auto it1 = test._lib_dirs.begin(), it2 = exp_lib.begin();
    it1 != test._lib_dirs.end() && it2 != exp_lib.end(); it1++, it2++) {
        EXPECT_TRUE(*it1 == *it2);
    }

}

TEST(TaskTest, WrongArg) {

    const char *argv1[] = { "test.exe", "file1", "-L" };
    EXPECT_THROW(TaskTest test(3, argv1), std::invalid_argument);

    const char *argv2[] = { "test.exe", "file1", "-L", "-c" };
    EXPECT_THROW(TaskTest test(4, argv2), std::invalid_argument);

    const char *argv3[] = { "test.exe", "file1", "-L", "-c",
    "file2" };
    EXPECT_THROW(TaskTest test(5, argv3), std::invalid_argument);

}