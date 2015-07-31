#ifndef TRIPLE_H
#define TRIPLE_H

#include "poly.h"

#define T Triple_t
#define P Poly_t

typedef struct T *T;

T Triple_new(P x, P y, P z);

P Triple_first(T t);

P Triple_second(T t);

P Triple_third(T t);

void Triple_setFirst(T, P);
void Triple_setSnd(T, P);
void Triple_setThird(T, P);


#undef T
#undef P


#endif
