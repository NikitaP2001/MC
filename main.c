#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <mc.h>
#include <pp.h>
#include <parser.h>

enum token_type {
        tok_keyword,
        tok_identifier,
        tok_constant,
        tok_strlit,
        tok_punctuator
};

struct token {
        struct list_head link;
        enum token_type type;


};

static struct pp_node*
pp_file_parse(struct pp_context *pp, char *file_name, bool is_global)
{
        struct pp_lexer lex;
        struct pp_parser parser;
        struct pp_node *node;
        struct fs_file *src = (is_global ?  fs_get_global(pp->fs, file_name) 
                : fs_get_local(pp->fs, file_name));
        if (src == NULL) {
                MC_LOG(MC_ERR, "Failed to open a file\n");
                return NULL;
        }

        pp_lexer_init(&lex, src);
        struct pp_token *token = NULL;
        while ((token = pp_lexer_get_token(&lex)))
                pp_lexer_add_token(&lex, token);
        assert(pp_lexer_noerror(&lex));

        struct pp_node *root_file = pp_node_create(pp_file);

        struct pp_token *tok_list = pp_lexer_result(&lex);
        assert(tok_list != NULL);
        pp_parser_init(&parser, tok_list);

        while ((node = pp_parser_fetch_node(&parser))) {
                if (!pp_node_file_insert(root_file, node)) {
                        MC_LOG(MC_ERR, "unexpected directive\n");
                        pp_node_destroy(node);
                        pp_token_list_destroy(tok_list);
                        tok_list = NULL;
                }
        }

        if (root_file->first_descendant == NULL || tok_list == NULL) {
                pp_node_destroy(root_file);
                root_file = NULL;
                if (tok_list != NULL)
                        pp_token_list_destroy(tok_list);
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
                strcpy(name, &name[1]);
                name[length - 1] = '\0';
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
        
        list_replace_range(gr_part, gr_part, first_gr_part, last_gr_part);

        struct pp_token *leftmost = pp_node_leftmost_leaf(gr_part);
        struct pp_token *rightmost = pp_node_rightmost_leaf(gr_part);

        struct pp_token *new_leftmode = pp_node_leftmost_leaf(first_gr_part);
        struct pp_token *new_rightmost = pp_node_rightmost_leaf(last_gr_part);

        list_replace_range(leftmost, rightmost, new_leftmode, new_rightmost);
        list_destroy_range(leftmost, rightmost, (list_free)pp_token_destroy);

        pp_node_destroy(gr_part);
}

static enum mc_status
pp_proc_include(struct pp_context *pp, struct pp_node *gr_part)
{
        enum mc_status status = MC_OK;
        struct pp_node *ctl_line = gr_part->first_descendant;
        /* should point on include token */
        struct pp_node *node = list_next(ctl_line->first_descendant);
        struct pp_node *inc_root;
        /* should point on a pp-tokens */
        node = list_next(ctl_line->first_descendant);
        assert(node->type == pp_tokens);

        node = node->first_descendant;
        assert(node->type == pp_leaf);
        if (node->leaf_tok->type == pp_hdr_name) {
                struct pp_token *hdr_name = node->leaf_tok;
                char *name = malloc(hdr_name->length);
                pp_token_getval(hdr_name, name);
                _Bool is_global = (name[0] == '<');
                if (pp_hdrname_to_filename(name, hdr_name->length)) {
                        inc_root = pp_file_parse(pp, name, is_global);
                        pp_include_on_node(pp, gr_part, inc_root);
                        pp_node_destroy(inc_root);
                }
                free(name);
        } else {
                PP_MSG(MC_CRIT, "macro expansion for the "
                "include directive is not suported");
                status = MC_FAIL;
        }

        return status;
}
#
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
enum mc_status 
pp_proc_ctl_line(struct pp_context *pp, struct pp_node *gr_part)
{
        enum mc_status status = MC_OK;
        struct pp_node *ctl_line = gr_part->first_descendant;
        if (pp_ctl_is_include(ctl_line)) {
                status = pp_proc_include(pp, gr_part);
        } else {
                MC_LOG(MC_DEBUG, "not supported ctl line");
        }

        return status;
}

enum mc_status
pp_proc_root(struct pp_context *pp)
{
        enum mc_status status = MC_OK;
        /* We expect at least one group with new-line char */
        struct pp_node *group = pp->root_file->first_descendant;
        assert(group != NULL);

        struct pp_node *gr_part = group->first_descendant;
        assert(gr_part != NULL);

        do {
                assert(gr_part->type == pp_group_part);

                struct pp_node *node = gr_part->first_descendant;
                switch (node->type) {
                        case pp_if_section:
                                MC_LOG(MC_DEBUG, "if section");
                                break;
                        case pp_control_line:
                                status = pp_proc_ctl_line(pp, gr_part);
                                MC_LOG(MC_DEBUG, "control line");
                                break;
                        case pp_text_line:
                                MC_LOG(MC_DEBUG, "text line");
                                break;
                        case pp_non_dir:
                                MC_LOG(MC_ERR, "unsupported directive");
                                break;
                        default:
                                MC_LOG(MC_CRIT, "unexpected node type");
                                break;
                }

        } while ((gr_part = list_next(gr_part)) && MC_SUCC(status));

        return status;
}

enum mc_status
pp_run(struct pp_context *pp, char *start_file)
{
        enum mc_status status = MC_OK;

        struct pp_node *root_file = pp_file_parse(pp, start_file, false);
        if (root_file == NULL) {
                MC_LOG(MC_CRIT, "root file %s parse fail", start_file);
                status = MC_FAIL;
        } else {
                pp->root_file = root_file;
                pp_proc_root(pp);
        }
        
        return status;
}

void pp_init(struct pp_context *pp, struct filesys *fs)
{
        pp->fs = fs;
        pp->root_file = NULL;
}

void pp_free(struct pp_context *pp)
{
        if (pp->root_file != NULL) {
                struct pp_token *first_tok = pp_node_leftmost_leaf(pp->root_file);
                pp_token_list_destroy(first_tok);
                pp_node_destroy(pp->root_file);
        }
}

int main()
{
        struct filesys fs;
        struct pp_context pp;

        fs_init(&fs);
        fs_add_local(&fs, "./mc/pp/");
        fs_add_local(&fs, "./mc"); 
        fs_add_local(&fs, "./"); 

        pp_init(&pp, &fs);

        enum mc_status status = pp_run(&pp, "parser.c");
        assert(MC_SUCC(status));

        pp_free(&pp);
        fs_free(&fs);
        puts("It`s not time yet");
}