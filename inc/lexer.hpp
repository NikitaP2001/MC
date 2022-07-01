#include "file.hpp"

namespace lexer {

enum token_tag {
    TOK_KEYWORD,
    TOK_IDENTIFIER,
    TOK_OPERATOR,

    TOK_CHAR,
    TOK_STRING,
    TOK_INTEGER,
    TOK_FLOAT,
};
extern const char *token_tag_table[];

/* ids of operators */
enum op_t {
    LEFT_RND_BR,        // (
    RIGHT_RND_BR,       // )
    LEFT_SQUARE_BR,     // [
    RIGHT_SQUARE_BR,    // ]
    ARROW,              // ->
    DOT,                // .
    INCREMENT,          // ++
    DECREMENT,          // --
    PLUS,               // +
    MINUS,              // -
    NOT,                // !
    INV,                // ~
    AMP,                // &
    MUL,                // *
    DIV,                // /
    PERCENT,            // %
    SHL,                // <<
    SHR,                // >>
    LESS,               // <
    LESS_EQ,            // <=
    GREATER,            // >
    GREATER_EQ,         // >=
    EQ,                 // ==
    NOT_EQ,             // !=
    CARET,              // ^
    BTW_OR,             // |
    OR,                 // ||
    AND,                // &&
    QUESTION,           // ?
    COLON,              // :
    ASSIGN,             // =
    ADD_ASSIGN,         // +=
    SUB_ASSIGN,         // -=
    MUL_ASSIGN,         // *=
    DIV_ASSIGN,         // /=
    REM_ASSIGN,         // %=
    SHL_ASSIGN,         // <<=
    SHR_ASSIGN,         // >>=
    AND_ASSIGN,         // &=
    XOR_ASSIGN,         // ^=
    OR_ASSIGN,          // |=
    COMA,               // ,
    RIGHT_CRL_BR,       // }
    LEFT_CRL_BR,        // {
    SEMICOLON,          // ;
};
extern const char *ops_sym_table[];

/* ids of keywords */
enum kw_t {
    KW_AUTO, 
    KW_DOUBLE,
    KW_INT,
	KW_STRUC,	
	KW_BREAK,
	KW_ELSE,
	KW_LONG,
	KW_SWITCH,	
	KW_CASE,
	KW_ENUM,
	KW_REGISTER,
	KW_TYPEDE,	
	KW_CHAR,
	KW_EXTERN,
	KW_RETURN,
	KW_UNIO,	
	KW_CONST,
	KW_FLOAT,
	KW_SHORT,
	KW_UNSIGNE,	
	KW_CONTINUE,
	KW_FOR,
	KW_SIGNED,
	KW_VOI,	
	KW_DEFAULT,
	KW_GOTO,
	KW_SIZEOF,
	KW_VOLATILE,	
	KW_DO,
	KW_IF,
	KW_STATIC,
	KW_WHILE,
};
extern const char *kws_sym_table[];

/*
 * stores information about single token
 * Note: in our case TU_content remains in memory
 * during all translation, so we do need to
 * allocate new memory for value fileld
 */
struct token {

    token_tag attribute;

    long line;

    union name {
        const char *str;
        const int id;

        name(int _id) : id(id) {}
        name(const char *szName) : str(szName) {}
    } t_name;

    token(token_tag tag, const char *szName)
    : t_name(szName) {}

    token(token_tag tag, int id)
    : t_name(id) {}

};

/* extracts tokens from unit contents
 */
bool lexer::get_tokens(snippet &TU, std::vector<token> &tokens);

// ::lexer