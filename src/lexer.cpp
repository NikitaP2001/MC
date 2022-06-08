#include <vector>
#include "lexer.hpp"

using namespace lexer;

const char *token_tag_table[] = 
{
    "keyword", "identifier", "operator",
    "char literal", "string literal",
    "integer literal", "floating point literal",
};

const char *ops_sym_table[] = {
    "(", ")", "[", "]", "->", ".",
    "++", "--", "+", "-", "!", "~",
    "&", "*", "/", "%", "<<", ">>",
    "<", "<=", ">", ">=", "==", "!=",
    "^", "|", "||", "&&", "?", ":",
    "=", "+=", "-=", "*=", "/=", "%=",
    "<<=", ">>=", "&=", "^=", "|=",
};


const char *kws_sym_table[] = {
    "auto",
    "double",
    "int",
    "struct",
    "break",        
    "else",        
    "long",       
    "switch",
    "case",         
    "enum",        
    "register",   
    "typedef",
    "char",         
    "extern",      
    "return",     
    "union",
    "const",        
    "float",       
    "short",      
    "unsigned",
    "continue",     
    "for",         
    "signed",     
    "void",
    "default",      
    "goto",        
    "sizeof",     
    "volatile",
    "do",           
    "if",          
    "static",     
    "while", 
};

bool get_tokens(std::vector<std::vector<char>> const &TU_content,
std::vector<token> &tokens)
{

    return 0;
}