#include <parser/symtable.h>
#include <pp/token.h>
#include <common.h>
#include <test_tools.h>

void parser_test_setup_snippet(struct parser_test_context *t_ctx,
			       const char *code_snippet)
{
	t_ctx->t_cfg.file_name = NULL;
	t_ctx->t_cfg.code_snippet = code_snippet;
}

void parser_test_setup_file(struct parser_test_context *t_ctx,
			    const char *file_name)
{
	t_ctx->t_cfg.file_name = file_name;
	t_ctx->t_cfg.code_snippet = NULL;
}

static void parser_context_init(struct parser_context *pctx,
				struct convert_context *cctx)
{
	pctx->curr = convert_get_token(cctx);
	pctx->last_error = NULL;
	pctx->last_pull = NULL;
}

static void parser_context_pull(struct parser_context *pctx)
{
	struct token *curr = pctx->curr;
	if (curr != NULL)
		pctx->curr = list_next(curr);
}

static struct token *pull_token(void *pp_data)
{
	struct parser_context *pctx = (struct parser_context *)pp_data;
	struct token *curr_tok = pctx->curr;
	parser_context_pull(pctx);
	return pctx->last_pull = curr_tok;
}

static void put_token(void *pp_data, struct token *tok)
{
	struct parser_context *pctx = (struct parser_context *)pp_data;
	/* Assumes tokens form a doubly-linked list */
	assert(list_next(tok) == pctx->curr); /* Verify assumption */
	pctx->curr = tok;
}

static struct token *fetch_token(void *pp_data)
{
	struct parser_context *pctx = (struct parser_context *)pp_data;
	return pctx->curr;
}

static mc_status_t parser_error_handler(void *pp_data, const char *message)
{
	struct parser_context *pctx = (struct parser_context *)pp_data;
	if (pctx->last_error != pctx->curr) {
		MC_DBG(MC_WARN, "Parser error callback: %s", message);
		pctx->last_error = pctx->curr;
	}
	return MC_FAIL;
}

static const char *parser_test_file_init(struct parser_test_context *t_ctx)
{
        struct parser_test_config *t_cfg = &t_ctx->t_cfg;
        const char *snippet = t_cfg->code_snippet;
        const char *file_name = NULL;
        if (snippet != NULL) {
                if (write_file(TFILE_NAME, snippet, strlen(snippet)))
                        file_name = TFILE_NAME;
        } else {
                file_name = t_cfg->file_name;
        }
        return file_name;
}

static void parser_test_file_free(struct parser_test_context *t_ctx)
{
        struct parser_test_config *t_cfg = &t_ctx->t_cfg;
        if (t_cfg->code_snippet != NULL)
                remove(TFILE_NAME);
}

mc_status_t parser_test_init(struct parser_test_context *t_ctx)
{
        const char *fname = parser_test_file_init(t_ctx);
        fs_init(&t_ctx->fs);
        fs_add_local(&t_ctx->fs, "./");
        pp_init(&t_ctx->pp, &t_ctx->fs);

        enum mc_status status = pp_run(&t_ctx->pp, fname);
        if (!MC_SUCC(status))
                return status;

        convert_init(&t_ctx->c_ctx, &t_ctx->pp);
        if (!MC_SUCC(convert_run(&t_ctx->c_ctx)))
                return status;

        parser_context_init(&t_ctx->p_ctx, &t_ctx->c_ctx);
        struct parser_clb ops = {
                .pull_token     = pull_token,
                .put_token      = put_token,
                .fetch_token    = fetch_token,
                .error          = parser_error_handler,
        };

	parser_test_file_free(t_ctx);
        parser_init(&t_ctx->parser, ops, &t_ctx->p_ctx);
        return status;
}

void parser_test_free(struct parser_test_context *t_ctx)
{
	parser_free(&t_ctx->parser);
	convert_free(&t_ctx->c_ctx);
        pp_free(&t_ctx->pp);
	fs_free(&t_ctx->fs);
}

mc_status_t parser_test_parse_translation_unit(struct parser_test_context *t_ctx,
					       struct pt_node **root)
{
	struct parser *ps = &t_ctx->parser;
	mc_status_t status = ps->ops->translation_unit(ps);

	if (!MC_SUCC(status))
		return status;

	*root = parser_result_pull(ps);
	if (*root == NULL)
		return MC_FAIL;

	return MC_OK;
}

void parser_test_free_tree(struct parser_test_context *t_ctx,
			   struct pt_node *root)
{
	if (root != NULL)
		pt_node_destroy(root);
	parser_test_free(t_ctx);
}

static struct pt_node *parser_test_find_node_by_symbol_impl(
	struct pt_node *node,
	enum parser_symbol sym,
	uint16_t occurrence,
	uint16_t *current)
{
	if (node == NULL)
		return NULL;

	if (node->sym == sym) {
		(*current)++;
		if (*current == occurrence)
			return node;
	}

	for (uint16_t index = 1; index <= pt_node_child_count(node); index++) {
		struct pt_node *child = pt_node_child_number(node, index);
		struct pt_node *result = parser_test_find_node_by_symbol_impl(
			child, sym, occurrence, current);
		if (result != NULL)
			return result;
	}

	return NULL;
}

struct pt_node *parser_test_find_node_by_symbol(struct pt_node *root,
						enum parser_symbol sym,
						uint16_t occurrence)
{
	uint16_t current = 0;
	return parser_test_find_node_by_symbol_impl(root, sym, occurrence,
		&current);
}

static struct token *parser_test_find_identifier_impl(struct pt_node *node,
						      const char *identifier,
						      uint16_t occurrence,
						      uint16_t *current)
{
	if (node == NULL)
		return NULL;

	if (node->sym == psym_identifier) {
		struct token *tok = pt_node_value_get(node);
		size_t id_len = strlen(identifier);

		if (tok != NULL && tok->type == tok_identifier
			&& tok->value.var_raw.length == id_len
			&& memcmp(tok->value.var_raw.value, identifier,
				id_len) == 0) {
			(*current)++;
			if (*current == occurrence)
				return tok;
		}
	}

	for (uint16_t index = 1; index <= pt_node_child_count(node); index++) {
		struct pt_node *child = pt_node_child_number(node, index);
		struct token *result = parser_test_find_identifier_impl(
			child, identifier, occurrence, current);
		if (result != NULL)
			return result;
	}

	return NULL;
}

struct token *parser_test_find_identifier(struct pt_node *root,
					  const char *identifier,
					  uint16_t occurrence)
{
	uint16_t current = 0;
	return parser_test_find_identifier_impl(root, identifier, occurrence,
		&current);
}