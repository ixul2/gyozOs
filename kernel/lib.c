#include "lib.h"

void *memset(void *dst, int c, size_t n)
{
    unsigned char *p = (unsigned char*)dst;

    for(size_t i = 0; i < n; i++)
        p[i] = (unsigned char)c;

    return dst;
}

void *memcpy(void *dst, const void *src, size_t n) {
  const char *s = (const char *)src;
  for (char *d = (char *)dst; n > 0; --n, ++s, ++d) {
    *d = *s;
  }
  return dst;
}

int strlen(char* s){
  int n = 0;
  while(s[n] != '\0'){
    n++;
  }
  return n;
}