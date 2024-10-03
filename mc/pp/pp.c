#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <pp.h>

static inline 
void pp_print_line_number(line_num_t line)
{
        mc_printf("    %lu | ", line);
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
        mc_putchar('\n');
        pp_print_line_number(file_line++);

        for ( ; chr_beg <= chr_end; chr_beg++) {
                mc_putchar(*chr_beg);
                if (*chr_beg == '\n' && chr_beg < chr_end)
                        pp_print_line_number(file_line++);

        }
        if (*chr_end != '\n')
                mc_putchar('\n');

}

void pp_print_token_line(struct pp_token *token)
{
        struct pp_token *tok_end = token;
        while (!pp_token_is_newline(tok_end) && list_has_next(tok_end)) {
                tok_end = list_next(tok_end);
        }
        pp_print_tokens(token, tok_end);
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
static inline 
struct pp_node*
pp_current_pos(struct pp_context *pp)
{
        return pp->curr_group_part;
}

static inline
void pp_set_pos(struct pp_context *pp, struct pp_node *node)
{
        pp->curr_group_part = node;
}

static inline 
struct pp_node*
pp_forward(struct pp_context *pp)
{
        pp->curr_group_part = list_next(pp->curr_group_part);
        return pp_current_pos(pp);
}

static struct pp_node*
pp_file_parse(struct pp_context *pp, char *file_name, bool is_global)
{
        struct pp_token *token = NULL;
        struct pp_lexer lex;
        struct pp_parser parser;
        struct pp_node *node;
        struct fs_file *src = (is_global ?  fs_get_global(pp->fs, file_name) 
                : fs_get_local(pp->fs, file_name));
        if (src == NULL) {
                pp_set_status(pp, fs_lasterr(pp->fs));
                return NULL;
        }

        if (!pp_lexer_init(&lex, src))
                return NULL;
        while ((token = pp_lexer_get_token(&lex)))
                pp_lexer_add_token(&lex, token);
        assert(pp_lexer_noerror(&lex));

        struct pp_node *root_file = pp_node_create(pp_file);

        struct pp_token *tok_list = pp_lexer_result(&lex);
        assert(tok_list != NULL);
        pp_parser_init(&parser, tok_list);

        while ((node = pp_parser_fetch_node(&parser))) {
                if (!pp_node_file_insert(root_file, node)) {
                        pp_error(pp, "unsupported directive");
                        pp_set_status(pp, MC_PARSE_ERROR);
                        pp_node_destroy(node);
                }
        }

        if (root_file->first_descendant == NULL 
                || !MC_SUCC(pp_get_status(pp))) {
                pp_token_list_destroy(tok_list);
                pp_node_destroy(root_file);
                root_file = NULL;
        }

        pp_lexer_free(&lex);
        return root_file;
}

static _Bool
pp_hdrname_to_filename(char *name, int length)
{
        _Bool status = false;
        assert(length >= 3);
        if ((name[0] == '<' && name[length - 1] == '>') ||
                (name[0] == '"' && name[length - 1] == '"')) {
                name[length - 1] = '\0';
                strcpy(name, &name[1]);
                status = true;
        }
        return status;
}

static void pp_include_on_node(struct pp_context *pp,
                               struct pp_node *gr_part, 
                               struct pp_node *root)
{
        struct pp_node *new_gr = root->first_descendant;
        assert(new_gr != NULL);

        /* pull from new root a range of group parts */
        struct pp_node *first_gr_part = new_gr->first_descendant;
        new_gr->first_descendant = NULL;
        assert(gr_part != NULL);
        struct pp_node *last_gr_part = new_gr->last_descendant;
        new_gr->last_descendant = NULL;

        /* for translation unit root structure */
        struct pp_node *pp_root = pp->root_file;
        struct pp_node *pp_gr = pp_root->first_descendant;

        /* make sure new group is valid */
        if (pp_gr->first_descendant == gr_part)
                pp_gr->first_descendant = first_gr_part;
        if (pp_gr->last_descendant == gr_part)
                pp_gr->last_descendant = last_gr_part;

        struct pp_token *leftmost = pp_node_leftmost_leaf(gr_part);
        struct pp_token *rightmost = pp_node_rightmost_leaf(gr_part);

        struct pp_token *new_leftmost = pp_node_leftmost_leaf(first_gr_part);
        struct pp_token *new_rightmost = pp_node_rightmost_leaf(last_gr_part);
        
        assert(!list_has_prev(first_gr_part) && !list_has_next(last_gr_part));
        list_replace_range(gr_part, gr_part, first_gr_part, last_gr_part);
        pp_set_pos(pp, first_gr_part);

        list_replace_range(leftmost, rightmost, new_leftmost, new_rightmost);
        list_destroy_range(leftmost, rightmost, (list_free)pp_token_destroy);
        pp_node_destroy(gr_part);
}

static void 
pp_proc_include(struct pp_context *pp, struct pp_node *ctl_line)
{
        struct pp_node *gr_part = pp_current_pos(pp);
        /* should point on include token */
        struct pp_node *node = list_next(ctl_line->first_descendant);
        struct pp_node *inc_root;
        /* should point on a pp-tokens */
        node = list_next(node);
        assert(node->type == pp_tokens);

        node = node->first_descendant;
        assert(node->type == pp_leaf);
        if (node->leaf_tok->type == pp_hdr_name) {
                struct pp_token *hdr_name = node->leaf_tok;
                char *name = malloc(hdr_name->length);
                pp_token_getval(hdr_name, name);
                _Bool is_global = (name[0] == '<');
                if (!pp_hdrname_to_filename(name, hdr_name->length)) {
                        pp_error(pp, "invalid header name format");
                        pp_set_status(pp, MC_SYNTAX_ERR);
                        return;
                }
                inc_root = pp_file_parse(pp, name, is_global);
                if (inc_root != NULL) {
                        pp_include_on_node(pp, gr_part, inc_root);
                        pp_node_destroy(inc_root);
                } else {
                        pp_error(pp, pp_str_status(pp));
                }
                        
                free(name);
        } else {
                pp_error(pp, "macro expansion for the "
                "include directive is not suported");
        }
}

static inline
 _Bool
pp_ctl_is_include(struct pp_node *ctl_line)
{
        struct pp_node *inc_dirc = list_next(ctl_line->first_descendant);
        return (pp_token_valcmp(inc_dirc->leaf_tok, PP_VAL_INCLUDE) == 0);
}

/* process next control line, changing the
 * preprocessor table state, and removing line
 * tokens from the list */
static inline
void
pp_proc_ctl_line(struct pp_context *pp, struct pp_node *ctl_line)
{
        if (pp_ctl_is_include(ctl_line)) {
                pp_proc_include(pp, ctl_line);
        } else {
                pp_error(pp, "not supported ctl line");
                pp_set_status(pp, MC_FAIL);
        }
}

static enum mc_status
pp_proc_root(struct pp_context *pp)
{
        struct pp_node *gr_part = pp_current_pos(pp);
       
        do {
                struct pp_node *node = gr_part->first_descendant;
                switch (node->type) {
                        case pp_if_section:
                                break;
                        case pp_control_line:
                                pp_proc_ctl_line(pp, node);
                                break;
                        case pp_text_line:
                                break;
                        case pp_non_dir:
                                MC_LOG(MC_ERR, "unsupported directive");
                                pp_print_node(node);
                                break;
                        default:
                                MC_LOG(MC_CRIT, "unexpected node type");
                                break;
                }

        } while ((gr_part = pp_forward(pp)) && MC_SUCC(pp_get_status(pp)) 
                && pp->err_count == 0);

        return pp_get_status(pp);
}

enum mc_status
pp_run(struct pp_context *pp, char *start_file)
{
        struct pp_node *root_file = pp_file_parse(pp, start_file, false);
        if (root_file == NULL) {
                pp_error(pp, "%s - %s", start_file, pp_str_status(pp));
                pp_set_status(pp, MC_OPEN_FILE_FAIL);
        } else {
                pp->root_file = root_file;
                /* We expect at least one group with new-line char */
                struct pp_node *node = root_file->first_descendant;
                assert(node != NULL);

                pp->curr_group_part = node->first_descendant;
                assert(pp->curr_group_part->type == pp_group_part);
                pp_proc_root(pp);
        }
        
        return pp_get_status(pp);
}

struct pp_token* pp_get_tu_start()
{
        return NULL;
}

void pp_init(struct pp_context *pp, struct filesys *fs)
{
        pp->fs = fs;
        pp->root_file = NULL;
        pp->curr_group_part = NULL;
        pp->last_err = MC_OK;
        pp->err_count = 0;
}

void pp_free(struct pp_context *pp)
{
        if (pp->root_file != NULL) {
                struct pp_token *first_tok 
                        = pp_node_leftmost_leaf(pp->root_file);
                pp_token_list_destroy(first_tok);
                pp_node_destroy(pp->root_file);
        }
}