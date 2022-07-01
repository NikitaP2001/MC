#include "preprocessor.hpp"
#include <file.hpp>

/* Removes comments in snippet, replacing it with spaces
 */
void purge_comments(snippet &sn)
{
	struct CommState {
        
        unsigned char state : 2;    // state of current symbol
                                    // less sign bit - slash comm
                                    // more sign bit - star comm

        unsigned char pr_state : 2;    // state of two previous symbols
    } comm = { 0 };
    
    for (auto *ln : sn.lines) {
        comm.pr_state = 0;
        auto *pc = ln->get_ptr();
        while (pc - ln->get_ptr() < ln->size()) {
            char chPr = *pc++;
            char ch = (pc - ln->get_ptr() < ln->size()) ? *pc : '\n';
            
            switch (ch) {
                case '\n':
                    if (chPr != '\\')
                        comm.state &= 10;
                    break;
                case '/':
                    if (!(comm.state & 0b10) && chPr == '/')
                        comm.state |= 0b1;
                    if (chPr == '*')
                        comm.state &= 0b1;						
                    break;
                case '*':
                    if (!(comm.state & 0b1) && chPr == '/')
                        comm.state |= 0b10;												
                    break;											
            }								
            if (comm.state || comm.pr_state) {
                int off = pc - ln->get_ptr();
                ln->write(" ", off - 1, 1);
                pc = ln->get_ptr() + off;
            }
            
            comm.pr_state <<= 1;
            comm.pr_state |= (comm.state >> 1) | comm.state;
        }
    }
}
