#ifndef TRACE_H
#define TRACE_H
#include <stdio.h>
#include "poly.h"

#define P Poly_t

P Trace_contains(char* s);

void Trace_addFunc(char* s);

#define Trace_TRACE(s, f, x, dox, r, dor)               \
    do {                                                \
        int exists = Trace_contains(s);                 \
        if (exists) {                                   \
            fprintf(stdout, "%s\n", "do args");         \
        }                                               \
        r = f x;                                        \
        if (exists) {                                   \
            fprintf(stdout, "%s\n", "do result");       \
        }                                               \
    }while(0)                                           





#undef P
#endif
