#include <string.h>
#include "string.h"

#define T String_t


#define MULTIPLIER 31
long String_hashCode(T s)
{
    long h=0;

    while(*s)
    {
        h = h*MULTIPLIER+(unsigned)*s++;
    }
    return h;
}

int String_equals(T x, T y)
{
    if (0 == strcmp((char*)x, (char*)y))
      return 1;

    return 0;
}


#undef T
