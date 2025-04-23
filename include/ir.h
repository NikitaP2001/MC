#ifndef _IR_H_
#define _IR_H_
#include <list.h>
#include <ir/object.h>
#include <ir/basic_block.h>
#include <ir/scalar.h>
#include <ir/function.h>
#include <ir/module.h>
#include <ir/value.h>
#include <ir/ins.h>
#include <ir/gen.h>
#include <parser/ast.h>

struct ir_value *ir_seek_var_in_table(struct hash_table *tbl, 
                                      struct token *name);


#endif /* _IR_H_ */