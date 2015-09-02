#include "assert.h"
#include "mem.h"
#include "hash.h"
#include "hash-set.h"
#include "error.h"

#define T HashSet_t
#define P Poly_t
#define K Poly_t

struct T {
    Hash_t hash;
};

// Dummy value to associate with const int `1` in the backing Map
static const int PRESENT = 1;

T HashSet_new(long (*hashCode) (P), Poly_tyEquals eq)
{
    T set;

    Mem_new(set);
    set->hash = Hash_new(hashCode, eq, NULL);

    return set;
}

/**
 * Returns <tt>true</tt> if this set contains the specified element.
 * More formally, returns <tt>true</tt> if and only if this set
 * contains an element <tt>e</tt> such that (o==null?e==null:o.equals(e)).
 *
 * @param o element whose presence in this set is to be tested
 * @return <tt>true</tt> if this set contains the specified element
 */
int HashSet_contains(T h, K x)
{
    Assert_ASSERT(h);

    return Hash_containsKey(h->hash, x);
}

/**
 * Adds the specified element to this set if it is not already present.
 * If this set already contains the element, the call leaves the set
 * unchanged and returns <tt>false</tt>.
 *
 * @param e element to be added to this set
 * @return <tt>true</tt> if this set did not already contain the specified
 * element
 */
int HashSet_add(T h, K k)
{
    Assert_ASSERT(h);
    if (HashSet_contains(h, k))
        return 0;

    return (Hash_put(h->hash, k, (P) PRESENT) == NULL);
}

/**
 * Removes the specified element from this set if it is present.
 * More formally, removes an element <tt>e</tt> such that
 * <tt>(o==null?e==null:o.equals(e))</tt>,
 * if this set contains such an element.  Returns <tt>true</tt> if
 * this set contained the element (or equivalently, if this set
 * changed as a result of the call).  (This set will not contain the
 * element once the call returns.)
 *
 * @param o object to be removed from this set, if present
 * @return <tt>true</tt> if the set contained the specified element
 */
int HashSet_remove(T h, K k)
{
    Assert_ASSERT(h);
    return ((const int) Hash_remove(h->hash, k) == PRESENT);
}

#undef T
#undef P
#undef K
