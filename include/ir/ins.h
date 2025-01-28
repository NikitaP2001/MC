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

        /* Signed value extent to type 
         * sextN <value result>, <value src>
         */
        ir_ins_sext16,
        ir_ins_sext32,
        ir_ins_sext64,

        /* Signed value extent to type 
         * sextN <value result>, <value src>
         */
        ir_ins_zext16,
        ir_ins_zext32,
        ir_ins_zext64,

        /* Truncate value to type
         * truncN <value result>, <value src>
         */
        ir_ins_trunc8,
        ir_ins_trunc16,
        ir_ins_trunc32,

        /* Float value extent to type
         * fpextN <value result>, <value src>
         */
        ir_ins_fpext64,
        ir_ins_fpext80,

        /* Float value truncate to type
         * fptruncN <value result>, <value src>
         */
        ir_ins_fptrunc32,
        ir_ins_fptrunc64,

        /* Binary or operation 
         * or <value result>, <operand 1>, <operand 2> 
         */
        ir_ins_or,

        /* Binary xor operation 
         * or <value result>, <operand 1>, <operand 2> 
         */
        ir_ins_xor,
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