
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

/* ids of operators */
enum op_t {
    LEFT_RND_BR,
    RIGHT_RND_BR,
};

/* ids of keywords */
enum kw_t {

};

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
        name(char *szName) : str(szName) {}
    } t_name;

    token(token_tag tag, char *szName)
    : t_name(szName) {}

    token(token_tag tag, int id)
    : t_name(id) {}

};

/* extracts tokens from unit contents
 */
bool get_tokens(std::vector<std::vector<char>> const &TU_content,
std::vector<token> &tokens);

} // ::lexer