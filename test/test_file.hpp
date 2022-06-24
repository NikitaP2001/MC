#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <gtest/gtest.h>
#include "file.hpp"

namespace fs = std::filesystem;



/* throw runtime error on attempt to read
 * empty file */
TEST(FileTest, ReadNotExist) {
    fs::path ftest = "testfile";
    if (fs::exists(ftest))
        fs::remove(ftest);

    EXPECT_THROW(mm_file rdfile(ftest.generic_string(), true), std::runtime_error);
    EXPECT_FALSE(fs::exists(ftest));
}

TEST(FileTest, CommonReadTest) {
    int fsize = 10000;
    int rd_cnt = 1000;
    fs::path ftest = "testfile";
    if (fs::exists(ftest))
        fs::remove(ftest);

    // generate random file content
    srand(time(NULL));
    char *fdat = new char[fsize];
    char *rdbuf = new char[fsize];
    for (int i = 0; i < fsize; i++)
        fdat[i] = static_cast<char>(rand() % 0xFF);

    std::ofstream stest(ftest.generic_string(), std::ios::binary);
    stest.write(fdat, fsize); 
    stest.close();
    fs::file_time_type lwr = fs::last_write_time(ftest);

    {
        mm_file fout(ftest.generic_string(), true);

        for (int i = 0; i < rd_cnt; i++) {
            int rdpos = rand() % fsize;
            int rdlen = rand() % fsize;
            
            rdlen = fout.read(&rdbuf[rdpos], rdpos, rdlen);
            for (int j = rdpos; j < rdlen; j++)
                ASSERT_EQ(rdbuf[j], fdat[j]);
        }
    }

    EXPECT_EQ(lwr, fs::last_write_time(ftest));

    EXPECT_NO_THROW(fs::remove(ftest));
    delete[] rdbuf;
    delete[] fdat;
}

TEST(FileTest, CommonWriteTest) {
    int fsize = 10000;
    int wr_cnt = 1000;
    fs::path ftest = "testfile";
    if (fs::exists(ftest))
        fs::remove(ftest);

    // generate random file content
    srand(time(NULL));
    char *fdat = new char[fsize];
    char *rdbuf = new char[fsize];
    for (int i = 0; i < fsize; i++)
        fdat[i] = static_cast<char>(rand() % 0xFF);

    std::ofstream stest(ftest.generic_string(), std::ios::binary);
    stest.write(fdat, fsize); 
    stest.close();

    {
        mm_file fout(ftest.generic_string(), false);

        for (int ntest; ntest < wr_cnt; ntest++) {
            int wrpos = rand() % fsize;
            int wrlen = rand() % (fsize - wrpos - 1) + 1;
            for (int i = 0; i < wrlen; i++) {
                char brnd = static_cast<char>(rand() % 0xFF);
                fdat[wrpos + i] = brnd;
            }
            fout.write(&fdat[wrpos], wrpos, wrlen);
        }

        fout.read(rdbuf, 0, fsize);
        for (int i = 0; i < fsize; i++)
            ASSERT_EQ(fdat[i], rdbuf[i]);
    }

    EXPECT_NO_THROW(fs::remove(ftest));
    delete[] rdbuf;
    delete[] fdat;
}

TEST(FileTest, ReadWriteTest) {
    int fsize = 10000;
    int test_cnt = 1000;
    fs::path ftest = "testfile";

    if (fs::exists(ftest))
        fs::remove(ftest);
    char *buf = new char[fsize];
    char *rdbuf = new char[fsize];

    {
        mm_file fout(ftest.generic_string(), false);
        for (int ntest = 0; ntest < test_cnt; ntest++) {
            int wrpos = rand() % fsize;
            int wrlen = rand() % (fsize - wrpos - 1) + 1;
            for (int i = 0; i < wrlen; i++) {
                char brnd = static_cast<char>(rand() % 0xFF);
                buf[i] = brnd;
            }
            fout.write(buf, wrpos, wrlen);
            int rdlen = fout.read(rdbuf, wrpos, wrlen);
            ASSERT_EQ(rdlen, wrlen);
            for (int j = 0; j < rdlen; j++)
                ASSERT_EQ(buf[j], rdbuf[j]);
        }
        
    }

    delete[] rdbuf;
    delete[] buf;
}