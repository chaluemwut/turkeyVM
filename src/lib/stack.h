#ifndef STACK_H
#define STACK_H

#include "list.h"
#include "poly.h"

#define T Stack_t
#define P Poly_t

typedef List_t Stack_t;

T Stack_new();

void Stack_push(T stk, P x);

P Stack_pop(T);

int Stack_isEmpty(T);

int Stack_size(T);

P Stack_seek(T);

#undef T
#undef P

#endif
