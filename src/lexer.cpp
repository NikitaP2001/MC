#include <vector>
#include <string>
#include <cctype>
#include "lexer.hpp"

using namespace lexer;

/* return true if only space-characters present
 * between pos and lastpos */
bool line_empty(char *pos, char *lastpos)
{
    while (pos <= lastpos)
        if (!isspace(*pos++))
            return false;
    return true;
}

/* Tokenize translation unit, which ought to 
 * be cleared of comments and preproc directives
 */
bool get_tokens(snippet &TU, std::vector<token*> &tokens)
{
    auto lnit = TU.lines.begin();
    while (lnit != TU.lines.end()) {
        line *ln = *lnit;
        char *pos = ln->get_ptr();
        char *lastpos = pos + ln->size();
        char c = '\0';

        // skip spaces
        while (pos <= lastpos) {
            if (!isspace(c = *pos++))
                break;
        }
        
        // end of line reached - no tok found
        if (!issymbol(c)) {
            lnit++;
            continue;
        }

        // named identifier
        if (isalpha(c) || c == '_') {
            std::string id = { c };

            while (pos <= lastpos) {
                c = *pos;

                // current line continues in next
                if (c == '\\' && line_empty(pos, lastpos)) {
                    if (++lnit == TU.lines.end()) {
                        std::cout << "invalid use of \\" << std::endl;
                        return false;
                    }
                    ln = *lnit;
                    pos = ln->get_ptr();
                    lastpos = pos + ln->size();
                    continue;
                }

                if (!isalpha(c) && !isdigit(c) && c != '_')
                    break;
                id.push_back(c);
                pos++;
            }

            // check as keyword
            for (int i = 0; i < sizeof(kw_sym_table)/sizeof(char*); i++) {
                if (kw_sym_table[i] == id.c_str()) {
                    token *ntok = new token(i, TOK_KEYWORD);
                    break;
                }

                // not found in kwds theb add as new id
                ntok->f_src= (*lnit)->f_ln_from;
                ntok->line = (*lnit)->get_number();
                tokens.push_back(ntok);
            }

        // string litaral
        } else if (c == '"' || c == '\''){
            std::string lit;
            char ltype = c;

            while (pos <= lastpos) {
                c = *pos;
                
                if (c == ltype) {
                    pos++;
                    break;
                }

                if (c == '\\') {

                    if (line_empty(pos, lastpos)) {
                        // escape new line
                        if (++lnit == TU.lines.end()) {
                            std::cout << "string literal reached EOF" << std::endl;
                            return false;
                        }
                        ln = *lnit;
                        pos = ln->get_ptr();
                        lastpos = pos + ln->size();
                        continue;

                    } else {
                        // excape next char
                        // note: line_empty() checked that not eol
                        c = *++pos;
                    }

                }

                lit.push_back(c):
                pos++;
            }

            if (ltype == '"') {

            } else {

            }

        } else if (isdigit(c)) {

        } else {

            do {
                switch (c) {
                }
            } while (pos <= lastpos);

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
