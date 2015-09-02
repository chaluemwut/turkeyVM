#include "stack.h"
#include "list.h"
#include "poly.h"
#include "error.h"

#define T Stack_t
#define P Poly_t

/**
 * @return a Stack_t
 */
T Stack_new()
{
    return List_new();
}

int Stack_isEmpty(T stk)
{
    return List_isEmpty(stk);
}

void Stack_push(T stk, P x)
{
    List_addFirst(stk, x);
    return;
}

/**
 * @return the top element
 */
P Stack_pop(T stk)
{
    if (List_isEmpty(stk))
        ERROR("try to pop on empty stack");

    return List_removeFirst(stk);
}

/**
 * NOTE: need traivals the list
 */
int Stack_size(T stk)
{
    return List_size(stk);
}

/**
 * get the top element but not pop
 */
P Stack_seek(T stk)
{
    return List_getIndexOf(stk, 0);
}
