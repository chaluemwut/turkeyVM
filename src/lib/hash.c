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
#define INIT_LOAD_FACTOR 0.75
#define INIT_CONTRACTION 0.25

#define T Hash_t
#define L List_t
#define E Triple_t
#define P Poly_t
#define K Poly_t
#define V Poly_t
#define S Status_t

typedef enum rehash_t
{
    TYPE_EXPANSION,
    TYPE_CONTRACTION
}Rehash_t;

typedef struct S *S;
struct S
{
    long insertions;//num of intert
    long deletes;
    long lookups;//num of lookup
    long links;//num of l->next
    long longest;//longest chain
    long maxSize;
    /*
     * maxLoad must small than INIT_LOAD_FACTOR
     */
    double maxLoad;
    long expansions;
    long contractions;
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
    double min_load;
    S status;
};


//static Status_t all = {0,0,0,0,0,0};

S Status_new()
{
    S s;
    
    Mem_new(s);

    s->insertions = 0;
    s->deletes = 0;
    s->lookups = 0;
    s->links = 0;
    s->longest = 0;
    s->maxSize = 0;
    s->maxLoad = 0;
    s->expansions = 0;
    s->contractions = 0;

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
    h->min_load = INIT_CONTRACTION;
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
    /*{{{*/
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
    if (load > h->status->maxLoad)
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
        link++;
        list = list->next;
    }

    //status-----------------------
    h->status->links+=link;
    if (link > h->status->longest)
      h->status->longest = link;
    //-----------------------------

    return NULL;
    /*}}}*/
}

/**
 * Tests if the specified object is a key in this hashtable.
 *
 * @param   key   possible key
 * @return  <code>true</code> if and only if the specified object
 *          is a key in this hashtable, as determined by the
 *          <tt>equals</tt> method; <code>false</code> otherwise.
 */
int Hash_containsKey(T h, K k)
{
    E e =  (E)Hash_containsCand(h, k, 0);

    return ((e)?1:0);
}

/**
 * Returns the value to which the specified key is mapped,
 * or {@code null} if this map contains no mapping for the key.
 *
 * <p>More formally, if this map contains a mapping from a key
 * {@code k} to a value {@code v} such that {@code (key.equals(k))},
 * then this method returns {@code v}; otherwise it returns
 * {@code null}.  (There can be at most one such mapping.)
 *
 * @param key the key whose associated value is to be returned
 * @return the value to which the specified key is mapped, or
 *         {@code null} if this map contains no mapping for the key
 */
V Hash_get(T h, K k)
{
    Assert_ASSERT(h);
    E e = (E)Hash_containsCand(h, k, 0);
    return ((e)?Triple_third(e):NULL);
}

/**
 * Expanded or contracted the capacity of and internally reorganizes this
 * hashtable, in order to accommodate and access its entries more
 * efficiently.  This method is called automatically when the
 * number of keys in the hashtable exceeds this hashtable's capacity
 * and load factor.
 *
 * @parm expansion(T) or sontraction(T)
 *
 */
static void rehash(T h, void(*d)(T))
{
    /*{{{*/
    L *oldBuckets;
    L *newBuckets;
    long oldSize;
    long newSize;
    long i;

    Assert_ASSERT(h);

    oldBuckets = h->buckets;
    oldSize = h->size;
    d(h);

    Mem_newSize(newBuckets, h->size);
    for (i=0; i<h->size; i++)
      *(newBuckets+i) = List_new();
    h->buckets = newBuckets;
    h->mask = h->size-1;

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
            List_addLast(*(h->buckets+idx), e);
            list = list->next;
        }
    }
    /*}}}*/
}

static void expansion(T h)
{
    h->status->expansions+=1;
    h->size = h->size*2;
}

static void contraction(T h)
{
    h->status->contractions+=1;
    h->size = h->size/2;
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
    {
        rehash(h, expansion);
    }

    return NULL;
    /*}}}*/
}

static int eleEquals(E x, E y)
{
    if (x == y)
      return 1;
    else 
      return 0;
}
/**
 * Removes the key (and its corresponding value) from this
 * hashtable. This method does nothing if the key is not in the hashtable.
 *
 * @param   key   the key that needs to be removed
 * @return  the value to which the key had been mapped in this hashtable,
 *          or <code>null</code> if the key did not have a mapping
 */
V Hash_remove(T h, K k)
{
    /*{{{*/
    Assert_ASSERT(h);
    long hcode;
    long bktIdx;
    L list;
    
    h->status->deletes++;
    E e = (E)Hash_containsCand(h, k, NULL);
    if (e)
    {

        hcode = (long)Triple_second(e);
        bktIdx = hcode&(h->mask);
        list = *(h->buckets+bktIdx);
        E ee = (E)List_remove(list, e, (Poly_tyEquals)eleEquals);
        h->numItems--;
        V r = Triple_third(ee);
        //XXX free(ee)
        
        if ((currentLoad(h) <= h->min_load)&&
                    (h->size > INIT_SIZE))
            rehash(h, contraction);
        return r;
    }
    else
    {
        return NULL;
    }
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
    printf("|Num of deletes:     %ld\n", h->status->deletes);
    printf("|Num of lookups:     %ld\n", h->status->lookups);
    printf("|Num of links:       %ld\n", h->status->links);
    printf("|Longest chain:      %ld\n", h->status->longest);
    printf("|Max hash size:      %ld\n", h->status->maxSize);
    printf("|Max load factor:    %lf\n", h->status->maxLoad);
    printf("|Expanded times:     %ld\n", h->status->expansions);
    printf("|Contraction times:  %ld\n", h->status->contractions);
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
#undef INIT_CONTRACTION
#undef T
#undef L
#undef K
#undef V
#undef P
#undef E
