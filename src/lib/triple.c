#include "triple.h"
#include "mem.h"
#include "assert.h"

#define T Triple_t
#define P Poly_t

struct T {
    P x;
    P y;
    P z;
};

T Triple_new(P x, P y, P z)
{
    T t;

    Mem_new(t);
    t->x = x;
    t->y = y;
    t->z = z;

    return t;
}

P Triple_first(T t)
{
    Assert_ASSERT(t);
    return t->x;
}

P Triple_second(T t)
{
    Assert_ASSERT(t);
    return t->y;
}

P Triple_third(T t)
{
    Assert_ASSERT(t);
    return t->z;
}

void Triple_setFirst(T t, P x)
{
    Assert_ASSERT(t);
    t->x = x;
}

void Triple_setSnd(T t, P y)
{
    Assert_ASSERT(t);
    t->y = y;

}

void Triple_setThird(T t, P z)
{
    Assert_ASSERT(t);
    t->z = z;
}

#undef T
#undef P
