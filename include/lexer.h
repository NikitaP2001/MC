#include <reader.h>

typedef struct lexer_token {       
        int id;
} lexer_token;

lexer_token *get_tokens(struct unit_reader *reader);
