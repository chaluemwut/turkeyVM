#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include "error.h"
#include "list.h"
#include "hash.h"
#include "triple.h"
#include "assert.h"


#define INIT_SIZE 4096
#define INIT_MASK 4095
#define INIT_LOAD_FACTOR 0.5

#define T Hash_t
#define L List_t
#define E Triple_t
#define P Poly_t
#define K Poly_t
#define V Poly_t
#define S Status_t

typedef struct S *S;
struct S
{
    long insertions;//num of intert
    long lookups;//num of lookup
    long links;//num of l->next
    long longest;//longest chain
    long maxSize;
    double maxLoad;
    long expanded;
    long shrink;
};

struct T
{
    L *buckets;
    long(*hashCode)(P);
    int(*equals)(P, P);
    void(*dup)(P, P);

    long numItems;
    long size;
    long mask;
    double load;
    S status;
};



//static Status_t all = {0,0,0,0,0,0};

S Status_new()
{
    S s;
    
    Mem_new(s);

    s->insertions = 0;
    s->lookups = 0;
    s->links = 0;
    s->longest = 0;
    s->maxSize = 0;
    s->maxLoad = 0;
    s->expanded = 0;
    s->shrink = 0;

    return s;
}

/**
 *
 *
 */
T Hash_new(long(*hashCode)(P)
            , int(*equals)(P, P)
            , void(*dup)(P, P))
{
    /*{{{*/
    long i;
    T h;

    Assert_ASSERT(hashCode);
    Assert_ASSERT(equals);

    Mem_new(h);
    Mem_newSize(h->buckets, INIT_SIZE);

    for (i=0; i<INIT_SIZE; i++)
      *(h->buckets+i) = List_new();
    h->hashCode = hashCode;
    h->equals = equals;
    h->dup = dup;
    h->numItems = 0;
    h->size = INIT_SIZE;
    h->mask = INIT_MASK;
    h->load = INIT_LOAD_FACTOR;
    h->status = Status_new();

    return h;
    /*}}}*/
}

/**
 *
 *
 */
static double currentLoad(T h)
{
    return 1.0*h->numItems/h->size;
}

/**
 * @return If contains k,return Triple_t which contains value.
 *      Otherwise, return NULL.
 *
 */
static E Hash_containsCand(T h, K k, K *result)
{
    long hcode;
    long bktIdx;
    long link = 0;
    long size;
    double load;
    L list;

    Assert_ASSERT(h);


    //status---------------------
    h->status->lookups++;
    size = h->size;
    if (size > h->status->maxSize)
      h->status->maxSize = size;
    load = currentLoad(h);
    if (load>h->status->maxLoad)
      h->status->maxLoad = load;
    //-----------------------------


    hcode = h->hashCode(k);
    bktIdx = hcode & h->mask;
    list = List_getFirst(*(h->buckets+bktIdx));
    while (list)
    {
        E e = (E)(list->data);
        K key = Triple_first(e);
        V value = Triple_third(e);
        
        link++;

        if (h->equals(k, key))
        {
            //status-----------------------
            h->status->links+=link;
            if (link > h->status->longest)
              h->status->longest = link;
            //-------------------------------

            if (result)
              *result = key;
            return e;
        }
        list = list->next;
    }

    //status-----------------------
    h->status->links+=link;
    if (link > h->status->longest)
      h->status->longest = link;
    //-----------------------------

    return NULL;
}

/**
 *
 */
int Hash_containsKey(T h, K k)
{
    E e =  (E)Hash_containsCand(h, k, 0);

    return ((e)?1:0);
}

/**
 *
 */
V Hash_get(T h, K k)
{
    Assert_ASSERT(h);
    E e = (E)Hash_containsCand(h, k, 0);
    return ((e)?Triple_third(e):NULL);
}

/**
 *
 */
static void putAllInternal(T h)
{
    /*{{{*/
    L *oldBuckets;
    L *newBuckets;
    long oldSize;
    long newSize;
    long i;

    Assert_ASSERT(h);

    h->status->expanded+=1;

    oldBuckets = h->buckets;
    oldSize = h->size;
    newSize = oldSize*2;
    Mem_newSize(newBuckets, newSize);
    for (i=0; i<newSize; i++)
      *(newBuckets+i) = List_new();
    h->buckets = newBuckets;
    h->size = newSize;
    h->mask = newSize-1;

    //traversal
    for (i=0; i<oldSize; i++)
    {
        L list;
        list = *(oldBuckets+i);
        list = List_getFirst(list);
        while (list)
        {
            E e = (E)(list->data);
            long hcode = (long)Triple_second(e);

            long idx = hcode & h->mask;
            List_addLast(*(newBuckets+idx), e);
            list = list->next;
        }
    }
    /*}}}*/
}

/**
 * @parm h the hashMap
 * @parm k key with which the specified value is to be associated
 * @parm v value to be associated with the specified key 
 *
 * @return the previous value associated with <tt>key</tt>, or
 *          <tt>null</tt> if there was no mapping for <tt>key</tt>.
 *
 */
V Hash_put(T h, K k, V v)
{
    /*{{{*/
    long hcode;
    long bktIdx;
    L list;
    P result;//key

    Assert_ASSERT(h);

    h->status->insertions++;
    //TODO deal with dup
    E e = (E)Hash_containsCand(h, k, &result);
    if (e)
    {
        if (h->dup)
        {
          h->dup(result, k);//call back deal with reduplicative  key.
          return NULL;
        }
        else
        {
            V r = Triple_third(e);
            Triple_setThird(e, v);
            return r;
        }
    }

    hcode = h->hashCode(k);
    bktIdx = hcode & (h->mask);
    list = *(h->buckets+bktIdx);
    List_addFirst(list, Triple_new(k, (P)hcode ,v));
    h->numItems++;

    if (currentLoad(h) >= h->load)
        putAllInternal(h);

    return NULL;
    /*}}}*/
}

static void Hash_foreach(T h, void(*d)(T, E, void(*)(K)), void(*f)(P))
{
    /*{{{*/
    Assert_ASSERT(h);
    Assert_ASSERT(f);

    long index;
    L list;

    for (index=0; index<h->size; index++)
    {
        list = List_getFirst(*(h->buckets+index));
        while (list)
        {
            E e = (E)(list->data);
            d(h, e, f);

            list = list->next;
        }
    }

    return;
    /*}}}*/
}

static void Hash_doKey(T h, E e, void(*f)(K))
{
    K key = Triple_first(e);
    f(key);

    return;
}

static void Hash_doValue(T h, E e, void(*f)(V))
{
    V value = Triple_third(e);
    f(value);

    return;
}

void Hash_foreachKey(T h, void(*f)(K))
{
    /*{{{*/
    Assert_ASSERT(h);
    Assert_ASSERT(f);

    Hash_foreach(h, Hash_doKey, f);

    /*}}}*/
}

void Hash_foreachValue(T h, void(*f)(V))
{
    Assert_ASSERT(h);
    Assert_ASSERT(f);

    Hash_foreach(h, Hash_doValue, f);
}

void Hash_status(T h)
{
    /*{{{*/
    Assert_ASSERT(h);
    printf("--------------------------------\n");
    printf("|Hash table status             |\n");
    printf("--------------------------------\n");
    printf("|Num of insertions:  %ld\n", h->status->insertions);
    printf("|Num of lookups:     %ld\n", h->status->lookups);
    printf("|Num of links:       %ld\n", h->status->links);
    printf("|Longest chain:      %ld\n", h->status->longest);
    printf("|Max hash size:      %ld\n", h->status->maxSize);
    printf("|Max load factor:    %lf\n", h->status->maxLoad);
    printf("|Expanded times:     %ld\n", h->status->expanded);
    printf("|Shrink times:       %ld\n", h->status->shrink);
    printf("|Averager position:  %lf\n", 
                1.0*h->status->links/h->status->lookups);
    printf("|Items nums:         %ld\n", h->numItems);
    printf("|currentLoad:        %lf\n", currentLoad(h));
    printf("--------------------------------\n");
    /*}}}*/
}


#undef INIT_SIZE
#undef INIT_MASK
#undef INIT_LOAD_FACTOR
#undef T
#undef L
#undef K
#undef V
#undef P
#undef E
