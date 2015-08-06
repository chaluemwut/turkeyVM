#ifndef HASHSET_H
#define HASHSET_H

#include "poly.h"

#define T HashSet_t
#define P Poly_t


typedef struct T *T;


T HashSet_new(long(*)(P), Poly_tyEquals);

int HashSet_contains(T, P);

int HashSet_add(T, P);

int HashSet_remove(T, P);



#undef T
#undef P
#endif
