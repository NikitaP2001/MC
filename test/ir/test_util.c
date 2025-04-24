#include "test_util.h"
#include <mc.h> /* Needed for MC_DBG, MC_ERR, MC_FAIL, etc. */
#include <parser/symtable.h>
#include <ir.h>
#include <stack.h>
#include <stdio.h> /* Keep for remove() */
#include <stdlib.h>
#include <string.h> /* For strlen, memset */
#include <token.h>
#include <pp/token.h>
#include <fs.h>
#include <list.h> /* For list_next */
#include <test_common.h>

/* --- Callbacks for Parser (Copied from test/parser/parser_test.c) --- */

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
        /* Keep test output clean, rely on test assertions */
        /* Use MC_DBG if parser errors need logging during tests */
        MC_DBG(MC_WARN, "Parser error callback: %s", message);
        pctx->last_error = pctx->curr;
    }
    return MC_FAIL; /* Propagate error */
}

/* --- IR Test Error Handler (Now using MC_DBG) --- */
mc_status_t test_irgen_error_handler(struct pt_node *node, const char *message)
{
    struct pp_token *pp_tok = NULL;
    const char *file_path = "<unknown>";
    unsigned long file_line = 0;

    /* Use MC_DBG with MC_ERR level for IR generation errors */
    if (node && node->node_value.value && node->node_value.value->first) {
        pp_tok = node->node_value.value->first;
        if (pp_tok->src_file) {
            file_path = pp_tok->src_file->path;
            file_line = (unsigned long)pp_tok->file_line;
            MC_DBG(MC_ERR, "IRGEN TEST ERROR: %s (near %s:%lu)", message, file_path, file_line);
        } else {
            MC_DBG(MC_ERR, "IRGEN TEST ERROR: %s (near token with no source file info)", message);
        }
    } else {
        MC_DBG(MC_ERR, "IRGEN TEST ERROR: %s (no node or token info available)", message);
    }

    return MC_FAIL; /* Propagate error */
}


/* --- New Full Environment Helper Functions --- */

mc_status_t ir_test_init_with_code(ir_test_full_env_t *full_env,
                   const char *code_snippet,
                   const char *module_name)
{
    mc_status_t status = MC_OK;
    struct parser_clb clb;

    /* Zero out the structure */
    memset(full_env, 0, sizeof(ir_test_full_env_t));

    /* 1. Filesystem Setup */
    fs_init(&full_env->fs);
    fs_add_local(&full_env->fs, "./");
    if (!write_file(TFILE_NAME, code_snippet, strlen(code_snippet))) {
        MC_DBG(MC_ERR, "Failed to write test code to %s", TFILE_NAME);
        status = MC_FAIL;
        goto fs_fail;
    }

    /* 2. Preprocessor Setup & Run */
    pp_init(&full_env->pp, &full_env->fs);
    status = pp_run(&full_env->pp, TFILE_NAME);
    if (!MC_SUCC(status)) {
        MC_DBG(MC_ERR, "Preprocessor failed for %s", TFILE_NAME);
        goto pp_fail;
    }

    /* 3. Token Conversion Setup & Run */
    convert_init(&full_env->convert_ctx, &full_env->pp);
    status = convert_run(&full_env->convert_ctx);
    if (!MC_SUCC(status)) {
        MC_DBG(MC_ERR, "Token conversion failed for %s", TFILE_NAME);
        goto convert_fail;
    }

    /* 4. Parser Setup */
    parser_context_init(&full_env->parser_ctx, &full_env->convert_ctx);
    clb.pull_token = pull_token;
    clb.put_token = put_token;
    clb.fetch_token = fetch_token;
    clb.error = parser_error_handler;
    parser_init(&full_env->parser, clb, &full_env->parser_ctx);

    /* 5. IR Generator Setup */
    irgen_init(&full_env->gen_ctx, &full_env->parser);
    full_env->gen_ctx.error = test_irgen_error_handler;

    /* 6. Get Module and Check Ops */
    full_env->module = full_env->gen_ctx.mod;
    if (!full_env->module) {
        MC_DBG(MC_ERR, "IR module not created by irgen_init");
        status = MC_FAIL;
        goto irgen_fail;
    }
    UNUSED(module_name);

    if (!full_env->gen_ctx.ops) {
        MC_DBG(MC_ERR, "IRGen Ops not initialized!");
        status = MC_FAIL;
        goto irgen_fail;
    }

    /* 7. Create Default Result Object */
    full_env->result_obj = irgen_obj_create(&full_env->gen_ctx, NULL);
    if (!full_env->result_obj) {
        MC_DBG(MC_ERR, "Failed to create result object");
        status = MC_FAIL;
        goto irgen_fail;
    }
    irgen_value_obj_set(&full_env->gen_ctx, full_env->result_obj);

    return MC_OK;

irgen_fail:
    parser_free(&full_env->parser);
convert_fail:
    convert_free(&full_env->convert_ctx);
pp_fail:
    pp_free(&full_env->pp);
fs_fail:
    fs_free(&full_env->fs);
    remove(TFILE_NAME);
    return status;
}

mc_status_t ir_test_parse_node(ir_test_full_env_t *full_env,
                   parser_process_node_t parse_func,
                   struct pt_node **out_node)
{
    mc_status_t status;

    if (!parse_func) {
        return MC_FAIL;
    }

    /* Ensure lookahead is NULL before parsing a specific fragment */
    if (full_env->parser.lookahead) {
        pt_node_destroy(full_env->parser.lookahead);
        full_env->parser.lookahead = NULL;
    }

    status = parse_func(&full_env->parser);
    if (!MC_SUCC(status)) {
        *out_node = NULL;
        return status; /* Parser error occurred */
    }

    *out_node = parser_result_pull(&full_env->parser);
    if (!*out_node) {
        /* This shouldn't happen if status was OK, but check anyway */
        MC_DBG(MC_ERR, "parser_result_pull returned NULL after successful parse");
        return MC_FAIL;
    }

    return MC_OK;
}

void ir_test_free(ir_test_full_env_t *full_env)
{
    /* Cleanup in reverse order of initialization */
    irgen_free(&full_env->gen_ctx); /* Handles module and result_obj cleanup */
    parser_free(&full_env->parser);
    convert_free(&full_env->convert_ctx);
    pp_free(&full_env->pp);
    fs_free(&full_env->fs);
    remove(TFILE_NAME); /* Clean up temp file */
}

/* ast_builder functions might still be useful for specific test cases */
/* Recursive helper for ast_destroy */
static void destroy_recursive(struct pt_node *node)
{
    if (!node)
        return;
    AST_FOREACH_CHILD(node) {
        struct pt_node *child_node = (struct pt_node *)entry;
        destroy_recursive(child_node);
    }
    pt_node_destroy(node);
}

void ast_destroy(struct pt_node *root)
{
    destroy_recursive(root);
}