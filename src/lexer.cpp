#include <vector>
#include <cctype>
#include "lexer.hpp"

using namespace lexer;

/* Removes comments in TU, replacing it with spaces
 */
static void purge_comments(std::vector<std::vector<char>> &TU_content)
{
	struct CommState {
        
        unsigned char state : 2;    // state of current symbol
                                    // less sign bit - slash comm
                                    // more sign bit - star comm

        unsigned char pr_state : 2;    // state of two previous symbols
    } comm = { 0 };
    
    for (auto &line : TU_content) {
        comm.pr_state = 0;
        auto ipos = line.begin();
        while (ipos != line.end()) {
                char chPr = *ipos++;
                char ch = (ipos != line.end()) ? *ipos : '\n';
                
                switch (ch) {
                    case '\n':
                        comm.state &= 0b10;
                        break;
                    case '/':
                        if (!(comm.state & 0b10) && chPr == '/')
                            comm.state |= 0b1;
                        if (chPr == '*')
                            comm.state &= 0b1;						
                        break;
                    case '*':
                        if (!(comm.state & 0b1) && chPr == '/')
                            comm.state |= 0b10;												
                        break;											
                }								
                if (comm.state || comm.pr_state)
                    *(ipos - 1) = ' ';
                
                comm.pr_state <<= 1;
                comm.pr_state |= (comm.state >> 1) | comm.state;
        }
    }
}

/* Tokenize translation unit, which ought to 
 * be cleared of comments and preproc directives
 */
bool lexer::get_tokens(std::vector<std::vector<char>> &TU_content,
std::vector<token> &tokens)
{
    purge_comments(TU_content);

    for (auto &line : TU_content) {
        /*

        // look pos iterator
        auto iLkPos = line.begin();

        for (auto itFwPos = iLkPos; itFwPos != line.end(); itFwPos++) {
            char lk = *iLkPos;

            if (isalpha(lk) || lk == '_') {
            } else {
                switch (lk) {

                }
            }
        }
        */

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