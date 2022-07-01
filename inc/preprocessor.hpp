/* 
 *  Preprocessor module forms the translation unit, execute
 *  preprocessor directives and erases comments
 * 
 *  Resulting TU will be represented as linked list of snippet
 *  structures 
 */

#include "file.hpp"


void purge_comments(snippet &sn);