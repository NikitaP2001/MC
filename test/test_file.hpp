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
    int wr_cnt = 5000;
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
        mm_file fout(ftest.generic_string(), false);

        for (int ntest = 0; ntest < wr_cnt; ntest++) {
            int wrpos = rand() % fsize;
            int wrlen;
            if (fsize - wrpos == 1)
                wrlen = 1;
            else
                wrlen = rand() % (fsize - wrpos - 1) + 1;
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
    srand(time(NULL));

    {
        mm_file fout(ftest.generic_string(), false);
        for (int ntest = 0; ntest < test_cnt; ntest++) {
            int wrpos = rand() % fsize;
            int wrlen;
            if (fsize - wrpos == 1)
                wrlen = 1;
            else
                wrlen = rand() % (fsize - wrpos - 1) + 1;
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

TEST(FileTest, Snippet) {
    int test_cnt = 1000;
    int max_ln_sz = 100;
    char fcon[] = "line1\nline2\nline3\nline4\n";
    fs::path ftest = "testfile";

    if (fs::exists(ftest))
        fs::remove(ftest);

    srand(time(NULL));
    std::ofstream stest(ftest.generic_string(), std::ios::binary);
    stest.write(fcon, sizeof(fcon)); 
    stest.close();

    {
        mm_file fin(ftest.generic_string(), true);
        snippet sn;
        fin.read(sn);
        // append 0 - term
        for (auto *ln : sn.lines) {
            ln->write("\0", ln->size() - 1, 1);
        }

        auto snit = sn.lines.begin();
        ASSERT_EQ(strcmp((*snit++)->get_ptr(), "line1"), 0);
        ASSERT_EQ(strcmp((*snit++)->get_ptr(), "line2"), 0);
        ASSERT_EQ(strcmp((*snit++)->get_ptr(), "line3"), 0);
        ASSERT_EQ(strcmp((*snit)->get_ptr(), "line4"), 0);

        // write test
        char *buf = new char[max_ln_sz];
        memset(buf, ' ', max_ln_sz);
        memmove(buf, "line1", sizeof("line1"));

        for (int nt = 0; nt < test_cnt; nt++) {
            int wrpos = rand() % max_ln_sz;
            int wrlen;
            if (max_ln_sz - wrpos == 1)
                wrlen = 1;
            else
                wrlen = rand() % (max_ln_sz - wrpos - 1) + 1;

            for (int i = 0; i < wrlen; i++)
                buf[wrpos + i] = static_cast<char>(rand() & 0xFF);

            sn.lines.front()->write(&buf[wrpos], wrpos, wrlen);
        }
        auto *pline = sn.lines.front()->get_ptr();
        for (int i = 0; i < sn.lines.front()->size(); i++)
            ASSERT_EQ(buf[i], pline[i]);


        // insert test
        snit = sn.lines.begin();
        std::advance(snit, 1);  // test for second line
        std::vector<char> vbuf(max_ln_sz, ' ');
        (*snit)->write("      ", 0, 6);

        for (int nt = 0; nt < test_cnt; nt++) {
            int wrpos = rand() % max_ln_sz;
            int wrlen;
            if (max_ln_sz - wrpos == 1)
                wrlen = 1;
            else
                wrlen = rand() % (max_ln_sz - wrpos - 1) + 1;

            for (int i = 0; i < wrlen; i++)
                buf[i] = static_cast<char>(rand() & 0xFF);
            
            vbuf.insert(vbuf.begin() + wrpos, buf, buf + wrlen); 
            (*snit)->insert(buf, wrpos, wrlen);

        }
        pline = (*snit)->get_ptr();
        for (int i = 0; i < (*snit)->size(); i++) {
            ASSERT_EQ(vbuf[i], pline[i]);
        }

        delete[] buf;
    }

    EXPECT_NO_THROW(fs::remove(ftest));
}