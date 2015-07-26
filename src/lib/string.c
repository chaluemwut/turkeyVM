#include <string.h>
#include <stdio.h>
#include "string.h"
#include "assert.h"
#include "mem.h"
#include "error.h"

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

T String_new(T x)
{
    int len;
    char* s;

    len = strlen(x);
    Mem_newSize(s, len+1);
    strcpy(s, x);

    if (String_equals(s, x))
      return s;

    ERROR("unknow");
}


#undef T
