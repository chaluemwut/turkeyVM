#include <stdlib.h>
#include "list.h"
#include "string.h"
#include "poly.h"
#include "assert.h"

#define L List_t
#define P Poly_t

static L traceList;

int Trace_contains(char* s)
{
    if (!traceList)
        traceList = List_new();

    P exist =  List_contains(traceList,
                s,
                (Poly_tyEquals)String_equals);
    return (exist)?1:0;
}

void Trace_addFunc(char* s)
{
    if (!traceList)
      traceList = List_new();

    List_addFirst(traceList, s);
}

#undef L
#undef P
