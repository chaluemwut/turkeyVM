#ifndef HASH_H
#define HASH_H

#include "list.h"
#include "poly.h"
#include "triple.h"

#define T Hash_t
#define P Poly_t
#define K Poly_t
#define V Poly_t
#define E Triple_t

typedef struct T *T;

T Hash_new(long (*hashCode) (P)
           , int (*equals) (P, P)
           , void (*dup) (P, P));

V Hash_put(T, K, V);

V Hash_remove(T, K);

int Hash_containsKey(T, K);

V Hash_get(T, K);

void Hash_foreachKey(T, void (*)(K));

void Hash_foreachValue(T, void (*)(V));

void Hash_status(T);

#undef T
#undef P
#undef K
#undef V
#undef E

#endif
