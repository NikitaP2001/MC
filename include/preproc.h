#ifndef _PREPROC_H_
#define _PREPROC_H_
#include <prunit.h>

/*

0. map encoding to internal translation encoding

1. delete escaped new line characters

2. decompose into preproc tokens.
No partial comm or preproc token in 
the end of source file.
Comment into one space.
*/



struct preproc_context {
        struct pr_unit *unit;
};

enum pr_tok_type {
        header_name,
        identifier,
        pp_number,
        char_constant,
        string_literal,
        punctuator,
        whitespace,
        other,
};

struct preproc_token {

        enum pr_tok_type type;

        struct snippet_iterator *stream_pos;
};



void preproc_run(struct pr_unit *unit);

_Bool pr_remove_newline_esc(struct pru_iter begin, struct pru_iter end);

#endif /* _PREPROC_H_ */