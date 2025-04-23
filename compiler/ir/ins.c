#include <ir/ins.h>
#include <ir/value.h>


void ir_ins_insert_use(struct instruction *ins, struct ir_value *val)
{
        struct ir_ins_use *use = calloc(1, sizeof(struct ir_ins_use));
        if (ins->use != NULL)
                list_insert(ins->use, use);
        ins->use = use;

        use->val = val;
        use->ins = ins;
        ir_value_add_use(val, ins->use);
}