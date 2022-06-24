#include <vector>
#include <string>
#include <gtest/gtest.h>
#include "../inc/lexer.hpp"

using namespace lexer;

TEST(GetTokenTest, SimpleMain) {

    std::string uin = " \
    // main func \
    int main() \
    { \
        int a = 0; \
        // call foo \
        foo(); \
        \
        \
        return 3; \
    } ";
    // expected tokens
    std::vector<token> etokens = {
        token(TOK_KEYWORD, KW_INT),
        token(TOK_IDENTIFIER, "main"),
        token(TOK_OPERATOR, LEFT_RND_BR),
        token(TOK_OPERATOR, RIGHT_RND_BR),
        token(TOK_OPERATOR, LEFT_CRL_BR),
        token(TOK_KEYWORD, KW_INT),
        token(TOK_IDENTIFIER, "a"),
        token(TOK_OPERATOR, ASSIGN),
        token(TOK_OPERATOR, SEMICOLON),
        token(TOK_IDENTIFIER, "foo"),
        token(TOK_OPERATOR, LEFT_RND_BR),
        token(TOK_OPERATOR, RIGHT_RND_BR),
        token(TOK_OPERATOR, SEMICOLON),
        token(TOK_OPERATOR, RIGHT_CRL_BR),
    };

    std::vector<std::vector<char>> unit;
    std::vector<token> tokens;

    ASSERT_EQ(0, get_tokens(unit, tokens));
    ASSERT_EQ(etokens.size(), tokens.size());
    for (auto it1 = tokens.begin(), it2 = etokens.begin();
    it1 != tokens.end(); it1++, it2++) {
        EXPECT_EQ(it1->attribute, it2->attribute);
        if (it1->attribute == TOK_IDENTIFIER) {
            EXPECT_STREQ(it1->t_name.str, it2->t_name.str);
        } else {
            EXPECT_EQ(it1->t_name.id, it2->t_name.id);
        }
    }
  
}