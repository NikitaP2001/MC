#include <vector>
#include <gtest/gtest.h>
#include "lexer.hpp"

using namespace lexer;

TEST(GetTokenTest, Zero) {
    std::vector<std::vector<char>> unit;
    std::vector<token> tokens;

    EXPECT_EQ(0, get_tokens(unit, tokens));
  
}