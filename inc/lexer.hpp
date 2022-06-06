#include <exception>

enum token_tag {
    KEYWORD,
    IDENTIFIER,
    OPERATOR,
    SEPARATOR,

    CHAR_LITERAL,
    STRING_LITERAL,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
};

struct token {

    token_tag tag;

    union value {
        long long *vint;
        double *vfloat;
        char *vstr;
    };

    token(char *_value, token_tag _tag)
    {
    
    }

    ~token()
    {

    }

};

class LexerError : public std::exception {
public:
    LexerError()
    {

    }

    char *what()
    {
        return "syntax error";
    }

};