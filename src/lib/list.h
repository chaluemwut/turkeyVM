#ifndef LIST_H
#define LIST_H

#include "poly.h"

#define T List_t
#define P Poly_t

typedef struct T *T;

struct List_t {
    P data;
    T next;
};


T List_new();

void List_addFirst(T,P);

void List_addLast(T, P);

int List_size(T);

P List_getIndexOf(T, int);

P List_contains(T, P, Poly_tyEquals f);



#undef T
#undef P

#endif
