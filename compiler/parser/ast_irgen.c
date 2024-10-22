#include <assert.h>
#include <ir.h>
#include <parser/ast.h>
#include <ir/irgen.h>

static mc_status_t irgen_statement(struct irgen_context *gen, 
                                   struct pt_node *stmt)
{
        mc_status_t status = MC_OK;
        UNUSED(gen);
        UNUSED(stmt);
        return status;
}

static mc_status_t irgen_block_item(struct irgen_context *gen, 
                                    struct pt_node *blk_itm)
{
        mc_status_t status = MC_PARSE_ERROR;

        struct pt_node *item = pt_node_child_first(blk_itm);
        if (item->sym == psym_declaration) {
                /* we do nothig, we will allocate var when used */
                status = MC_OK;
        } else if (item->sym == psym_statement) {
                status = irgen_statement(gen, item);
        }
        return status;
}

static mc_status_t irgen_block_item_list(struct irgen_context *gen, 
                                         struct pt_node *blk_itm_lst)
{
        mc_status_t status = MC_PARSE_ERROR;
        AST_FOREACH_CHILD(blk_itm_lst) {
                struct pt_node *blk_itm = (struct pt_node *)entry;
                status = irgen_block_item(gen, blk_itm);
                if (!MC_SUCC(status))
                        break;
        }
        return status; 
}

static mc_status_t irgen_compound_statement(struct irgen_context *gen, 
                                            struct pt_node *comp_stmt)
{
        mc_status_t status = MC_OK;
        assert(comp_stmt->sym == psym_compound_statement);
        if (pt_node_child_count(comp_stmt) != 0) {
                struct pt_node *blk_itm_lst = pt_node_child_first(comp_stmt);
                status = irgen_block_item_list(gen, blk_itm_lst);
        }
        return status; 
}

static mc_status_t irgen_function_definition(struct irgen_context *gen, 
                                            struct pt_node *func_def)
{
        mc_status_t status = MC_PARSE_ERROR;
        /* TODO: check func def constraints 6.9.1 */

        struct basic_block *bb_old = irgen_bb_create(gen, func_def);
        status = irgen_compound_statement(gen, pt_node_child_last(func_def));
        assert(bb_old == NULL);
        irgen_bb_restore(gen, bb_old);

        return status;
}

static mc_status_t irgen_external_declaration(struct irgen_context *gen, 
                                              struct pt_node *ex_decl)
{
        mc_status_t status = MC_PARSE_ERROR;
        assert(ex_decl->sym == psym_external_declaration);
        struct pt_node *decl = pt_node_child_first(ex_decl);
        switch (decl->sym) {
                case psym_function_definition:
                        irgen_function_create(gen, decl);
                        status = irgen_function_definition(gen, decl);
                        break;
                case psym_declaration:
                        /* we already done with it in parser.c */
                        status = MC_OK;
                        break;
                default:
                        MC_LOG(MC_CRIT, "unexpected node type");
                        break;
        }
        return status;
}

mc_status_t irgen_translation_unit(struct irgen_context *gen, 
                                      struct pt_node *unit)
{
        mc_status_t status;
        assert(unit->sym == psym_translation_unit);

        AST_FOREACH_CHILD(unit) {
                struct pt_node *ext_decl = (struct pt_node *)entry;
                status = irgen_external_declaration(gen, ext_decl);
                if (!MC_SUCC(status))
                        goto fail;
        }
        
        return status;
fail:
        irgen_free(gen);
        return status;
}