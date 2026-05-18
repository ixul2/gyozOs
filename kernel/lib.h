#ifndef _LIB_H_
#define _LIB_H_

#include <stddef.h>

void *memset(void *dst, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
int strlen(char *s);
void *strcpy(char *dst, const char *src);

#endif
