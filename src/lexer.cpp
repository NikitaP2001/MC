#include <vector>
#include <cctype>
#include "lexer.hpp"

using namespace lexer;

/* Tokenize translation unit, which ought to 
 * be cleared of comments and preproc directives
 */
bool lexer::get_tokens(snippet &TU,
std::vector<token> &tokens)
{
    for (auto &ln : TU.lines) {
        int pos = 0;
        int prevpos = 0;

        while (pos < ln->size()) {
            char c = *(ln->get_ptr() + pos); 

            if (isspace(c)) {

            }
        }

    }

    return 0;
}

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
    ",", "}", "{", ";",
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