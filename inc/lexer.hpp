
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
    AUTO, 
    DOUBLE,
    INT,
	STRUC,	
	BREAK,
	ELSE,
	LONG,
	SWITC,	
	CASE,
	ENUM,
	REGISTER,
	TYPEDE,	
	CHAR,
	EXTERN,
	RETURN,
	UNIO,	
	CONST,
	FLOAT,
	SHORT,
	UNSIGNE,	
	CONTINUE,
	FOR,
	SIGNED,
	VOI,	
	DEFAULT,
	GOTO,
	SIZEOF,
	VOLATIL,	
	DO,
	IF,
	STATIC,
	WHILE,
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
bool get_tokens(std::vector<std::vector<char>> &TU_content,
std::vector<token> &tokens);

} // ::lexer