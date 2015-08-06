#ifndef TRACE_H
#define TRACE_H
#include <stdio.h>
#include "poly.h"

void Trace_indent();

void Trace_unindent();

void Trace_spaces();

int Trace_contains(char* s);

void Trace_addFunc(char* s);

#define Trace_TRACE(s, f, x, dox, r, dor)               \
    do {                                                \
        int exists = Trace_contains(s);                 \
        if (exists) {                                   \
            dox x;                                      \
        }                                               \
        r = f x;                                        \
        if (exists) {                                   \
            dor r;                                      \
        }                                               \
    }while(0)                                           


#define Trace_Stack(s, f, x, dox)                       \
    do {                                                \
        int exists = Trace_contains(s);                 \
        if (exists) {                                   \
            dox x;                                      \
        }                                               \
        f x;                                            \
        if (exists) {                                   \
            dox x;                                      \
        }                                               \
    }while(0)            


#endif
