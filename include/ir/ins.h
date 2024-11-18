#ifndef _IR_INS_H_
#define _IR_INS_H_
#include <list.h>

enum ir_ins_type {

        /* control flow instruction 
         * ctf <bool cond> <bb 1> <bb 2>
         * OR
         * ctf <bb 1> in unconditional transfer case */
        ir_ins_ctf,
        /* mov <value dest>, <value src> */
        ir_ins_mov,

        /* cmp <value result>, <value 1>, <value 2> */
        ir_ins_cmp,
};

struct value;
struct instruction;

struct ir_ins_use {
        struct list_head *link;
        struct value *val;
        struct instruction *ins;
};

struct instruction {
        enum ir_ins_type type;
        struct ir_ins_use *use;
};

void ir_ins_insert_use(struct instruction *ins, struct value *val);

static inline _Bool ir_ins_is_control_transfer(struct instruction *ins)
{
        return (ins->type == ir_ins_ctf);
}

#endif /* _IR_INS_H_ */