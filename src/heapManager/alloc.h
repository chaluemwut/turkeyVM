#ifndef ALLOC_H
#define ALLOC_H

#include "../main/turkey.h"

#define C Class_t
#define O Object_t


#define OBJECT_DATA(obj, index, type) (*((type*)(obj->data+(index))))
#define ARRAY_DATA(obj, index, type) (*((type*)obj->data+(index)))
#define ARRAY_IDX(obj, index, type) (((type*)obj->data)+(index))


#define CHECK_ARRAY(array, index)                   \
    do{                                             \
        Assert_ASSERT(array);                       \
        Assert_ASSERT(array->type == OBJECT_ARRAY);   \
        if (index < 0 || index >= array->length)    \
            throwException("OutofArrayBound");      \
    }while(0)

typedef struct O *O;

typedef enum
{
    OBJECT_OBJECT,
    OBJECT_ARRAY,
    OBJECT_STRING,
}Object_e;

//FIXME enhancement
typedef enum
{
    TODO
}Array_type;

struct O
{
    struct C* class;//refer to the methodArea.
    struct classblock* cb;
    unsigned int* data;
    //-------------------------
    Object_e type;
    int length;//If it's array, it the arraylength, otherwise, it's 0.
    int atype;//array type
    //----------------------------------------------
    /*
     * The bingding is for the head of every Class. When reflected, 
     * find the responding Class throgh the bingding.Normal Object's
     * bingding is NULL, but for java/lang/Class Object, it's bingding 
     * point to the MethodArea.
     * NOTE: in class.c
     * @qcliu 2015/03/23
     */
    struct C* binding;

    int el_size;//normal object's el_size is 4B
    int copy_size;//for turkeyCopy(), assign in alloc.c
};


/*Alloc*/

extern void* sysMalloc(int n);

extern O allocObject(struct C* class);

extern O allocTypeArray(int type, int size, char* element_name);

extern O allocArray(struct C* class, int size, int el_size, int atype);

extern void dumpObject(O);

#undef C
#undef O

#endif
