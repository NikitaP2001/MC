#include <assert.h>

#include <pp.h>

static inline 
void pp_print_line_number(line_num_t line)
{
        printf("    %lu | ", line);
}

static void
pp_print_tokens(struct pp_token *tok_beg, struct pp_token *tok_end)
{
        assert(tok_beg->src_file == tok_end->src_file);
        const char *chr_beg = tok_beg->value;
        const char *chr_end = tok_end->value;
        const char *chr_first = fs_file_read(tok_beg->src_file);
        line_num_t file_line = pp_token_line(tok_beg);

        while (chr_beg != chr_first) {
                if (*chr_beg == '\n') {
                        chr_beg += 1;
                        break;
                }
                chr_beg -= 1;
        }
        pp_print_line_number(file_line++);

        for ( ; chr_beg <= chr_end; chr_beg++) {
                putchar(*chr_beg);
                if (*chr_beg == '\n' && chr_beg < chr_end)
                        pp_print_line_number(file_line++);

        }
        if (*chr_beg != '\n')
                putchar('\n');

}

void pp_print_node(struct pp_node *node)
{
        struct pp_token *tok_beg = pp_node_leftmost_leaf(node);
        struct pp_token *tok_end = pp_node_rightmost_leaf(node);
        pp_print_tokens(tok_beg, tok_end);
}

void pp_error(struct pp_context *pp, const char *format, ...)
{
        va_list args;
        struct pp_token *tok_beg;
        pp->err_count += 1;

        struct pp_node *gr_part = pp->curr_group_part;
        if (gr_part != NULL) {
                tok_beg = pp_node_leftmost_leaf(gr_part);
                printf("%s:%lu: ", fs_file_path(tok_beg->src_file), 
                        pp_token_line(tok_beg));
        }
        
        va_start(args, format);
        vprintf(format, args);
        putchar('\n');

        if (gr_part != NULL)
                pp_print_node(gr_part);

        va_end(args);
}