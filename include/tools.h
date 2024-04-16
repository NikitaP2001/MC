#ifndef _TOOLS_H_
#define _TOOLS_H_
#include <stdint.h>

#define PEARSON_TABLE_SIZE 256

uint8_t pearson_hash(const char *data, size_t length);

static inline int64_t max(int64_t num1, int64_t num2)
{
        return (num1 > num2 ? num1 : num2);
}

#endif /* _TOOLS_H_ */