#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "poly.h"
#include "list.h"
#include "mem.h"
#include "error.h"


#define T List_t
#define P Poly_t


T List_new() 
{
    T l;
    Mem_new(l);
    l->data = 0;
    l->next = 0;

    return l;
}


/**
 * @parm 
 *      x data
 * @parm
 *      l next pointer
 *
 * @return a new List_t struct 
 */
static T List_newNode(P x, T l) 
{
    T p;
    Mem_new(p);
    p->data = x;
    p->next = l;

    return p;
}

/**
 * @parm l list head
 * @parm x poly type data
 * 
 * NOTE: we use head->data to record tail
 */
void List_addFirst(T l, P x) 
{
    T t;

    t = List_new();
    t = List_newNode(x, l->next);
    l->next = t;

    if (l->data == NULL) {
        l->data = t;
    }
    return;
}

/**
 *
 *
 */
void List_addLast(T l, P x) 
{
    T tail, p;

    if (l->next == NULL) {
        List_addFirst(l, x);
        return;
    } 

    tail = (T)l->data;
    p = List_newNode(x, NULL);
    tail->next = p;
    l->data = p;

    return;
}


/**
 * need traivals the list
 */
int List_size(T l) 
{
    T p;
    int i = 0;
    p = l->next;
    while (p) {
        i++;
        p = p->next;
    }

    return i;
}

int List_isEmpty(T l)
{
    return (0==l->next);
}

/**
 * @return if not found or index > size, return 0
 */
P List_getIndexOf(T l, int index) 
{
    T p = l->next;
    if (index < 0) {
        ERROR("invalid argument");
        return 0;
    }
    while (p) {
        if (0 == index)
            return p->data;//found!
        index--;
        p=p->next;
    }
    return 0;
}

/**
 * return the first fit
 *
 */
P List_contains(T l, P x, Poly_tyEquals f)
{
    T p;

    p = l->next;
    while (p) {
        if (f(x, p->data))
          return p->data;

        p = p->next;
    }

    return 0;
}

/**
 *
 *
 */
P List_removeFirst(T l)
{
    T p;

    if (0 == l->next)
      ERROR("List size is 0\n"); 

    p = l->next;
    l->next = l->next->next;
    //XXX omit the l->data. 

    return p->data;

}



#undef T
#undef P
