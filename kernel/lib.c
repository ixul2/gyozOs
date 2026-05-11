#include "lib.h"

void *memset(void *dst, int c, size_t n)
{
    unsigned char *p = (unsigned char*)dst;

    for(size_t i = 0; i < n; i++)
        p[i] = (unsigned char)c;

    return dst;
}