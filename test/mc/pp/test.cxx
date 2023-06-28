#include <string>
#include <vector>
#include <tuple>
#include <fstream>
#include <stdexcept>
#include <stdbool.h>
#include <stdlib.h>

#include <unistd.h>

#include <gtest/gtest.h>

extern "C" {
        #include <pp.h>
        #include <fs.h>
        #include <tools.h>
        #include <testool.h>
}

static constexpr const char *g_kTestSrcName = "file.e";



class SrcFile {
public:
/* in form of expected <tok type, file text, expected text> */

class token {
using token_val = std::tuple<pp_type, std::string, std::optional<std::string>>;
public:

        token(pp_type expectedType, std::string fileText)
        : m_value(std::make_tuple(expectedType, fileText, std::nullopt))
        {

                
        }

        token(pp_type expectedType, std::string fileText, std::string expectedText)
        : m_value(std::make_tuple(expectedType, fileText, expectedText))
        {

        }

        pp_type getToken() const
        {
                return std::get<0>(m_value);
        }

        std::string getFileText() const
        {
                return std::get<1>(m_value);
        }

        std::string getTokenText() const
        {
                if (std::get<2>(m_value).has_value())
                        return std::get<2>(m_value).value();
                else
                        return getFileText();
        }

private:

token_val m_value;

};

public:
        SrcFile(std::string fName = g_kTestSrcName)
        : m_fileName(fName)
        {
                fs_init(&m_fs);
                fs_add_local(&m_fs, "./");
        }

        struct fs_file *readContent()
        {
                struct fs_file *src = fs_get_local(&m_fs, m_fileName.c_str());
                return src;
        }

        void writeTokens(std::vector<token> &tokens) 
        {
                std::fstream fs(m_fileName, std::ios::out);
                auto strToks = tokensToStr(tokens);
                fs.write(strToks.c_str(), strToks.size());
                fs.close();
                assert(!fs.fail());
        }

        ~SrcFile()
        {
                fs_free(&m_fs);
                remove(m_fileName.c_str());
        }

private:

        std::string tokensToStr(std::vector<token> &tokens)
        {
                std::string result;
                for (const auto &tok : tokens)
                        result.append(tok.getFileText());

                return result;
        }

private:

const std::string m_fileName;

struct filesys m_fs = {};
};

class PpTest {

public:

        PpTest(std::vector<SrcFile::token> &&tokens)
                : m_tokens(tokens)
        {
                m_sf.writeTokens(tokens);
        }

        void run()
        {
                struct preproc pp = {};
                struct fs_file *src = m_sf.readContent();
                pp_init(&pp, src);
                expectTokensEqPreproc(pp);
                pp_free(&pp);
        }

private:

        void expectTokensEqPreproc(struct preproc &pp) 
        {
                struct pp_token *token = NULL;
                auto itoks = m_tokens.begin();
                while ((token = pp_get_token(&pp)) != NULL && itoks != m_tokens.end()) {
                        EXPECT_EQ(pp_token_valcmp(token, itoks->getTokenText().c_str()), 0);
                        EXPECT_EQ(token->type, itoks->getToken());
                        std::advance(itoks, 1);
                        pp_add_token(&pp, token);
                }
                EXPECT_EQ(itoks, m_tokens.end());
        }

SrcFile m_sf;

std::vector<SrcFile::token> m_tokens;

};


TEST(pp_test, header_name)
{
        PpTest pp({
                { pp_punct, "#" },
                { pp_id, "include  ", "include"},
                { pp_hdr_name, "<h_dr/name.cc>"},
                { pp_other, "\n"},
        });
        pp.run();
}


TEST(pp_test, header_name_multiline)
{
        PpTest pp({
                { pp_punct, "#" },
                { pp_id, "include ", "include"},
                { pp_punct, "<"},
                { pp_id, "file"},
                { pp_other, "\n"},
                { pp_punct, "> ", ">"},
                { pp_other, "\n"},
        });
        pp.run();
}

TEST(pp_test, header_name_escline)
{
        PpTest pp({
                { pp_punct, "#" },
                { pp_id, "include ", "include"},
                { pp_hdr_name, "<h_dr/n\\\name.cc>", "<h_dr/name.cc>"},
                { pp_other, "\n"},
                
        });
        pp.run();
}

TEST(pp_test, str_lit_oneline)
{
        PpTest pp({
                { pp_str_lit, "\"lit 943\\123\""},
                { pp_str_lit, "\"1 line\""},
                { pp_other, "\n"},
                { pp_str_lit, "\"2 line\""},
        });
        pp.run();
}

TEST(pp_test, str_lit_multline)
{
        PpTest pp({
                { pp_str_lit, "\"1 line\\\n2 line\"", "\"1 line2 line\""}
        });
        pp.run();
}

TEST(pp_test, comment_star_oneline)
{
        PpTest pp({
                { pp_punct, "#" },
                { pp_id, "define /* abcde */", "define"},
                { pp_id, "SOME_DEF ", "SOME_DEF"},
                { pp_number, "1233"},
                { pp_other, "\n"},
                
        });
        pp.run();
}


GTEST_API_ int main(int argc, char **argv) {
  printf("Running PP tests from %s\n", __FILE__);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}