#ifndef _FNV_1_H_
#define _FNV_1_H_

#include <stddef.h>

#include <mc/types.h>

#define FNV_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_PRIME        0x00000100000001b3

inline static hash_key_t fnv_1_hash(const char *string, size_t length)
{
        hash_key_t hash = FNV_OFFSET_BASIS;
        for (size_t i = 0; i < length; i++)
                hash = (hash * FNV_PRIME) ^ string[i];
        return hash;
}

#endif /* _FNV_1_H_ */