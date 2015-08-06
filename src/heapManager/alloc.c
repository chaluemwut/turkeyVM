/*------------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  alloc.c                                */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2014/12/31                             */
/*  DESCRIPTION           :  for the javaVM                         */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 *
 *//*}}}*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../classloader/class.h"
#include "../util/exception.h"
#include "../main/turkey.h"
#include "../lib/error.h"

#define C Class_t
#define O Object_t

extern int vmsize;
extern C primClass[];

static const int ObjHeaderSize = sizeof(struct O);

void* sysMalloc(int n)
{
    void* mem = malloc(n);

    if(mem == NULL)
      ERROR("malloc error");

    memset(mem, 0, n);

    vmsize += n;
    return mem;
}


/*
 * The object's isArray is FALSE, so that the length is 0.
 *
 * NOTE: obj->class refer to the java/lang/Class Object whitch
 *      in the methodArea. When a obj.getClass() it's return the
 *      relevent java/lang/Class Object.
 * @qcliu 2015/03/23
 */
O allocObject(C class)
{
    ClassBlock* cblock = CLASS_CB(class);
    int obj_size = cblock->obj_size;
    O obj = (O)sysMalloc(ObjHeaderSize + sizeof(int)*obj_size);
    //---------------------------------
    obj->isArray = FALSE;
    obj->length = 0;
    obj->atype = 0;
    //--------------------------
    obj->class = class;
    obj->binding = NULL;
    obj->data = (unsigned int*)(obj+1);
    memset(obj+1, 0, sizeof(int) * obj_size);

    obj->cb = cblock;
    obj->el_size = sizeof(int);
    obj->copy_size = ObjHeaderSize+ sizeof(int)*obj_size;

    return obj;

}

/*
 *The Array's isArray is TRUE, and the length is arraylength.
 * @qcliu 2015/03/24
 */
O allocArray(C class, int size, int el_size, int atype)
{
    O obj;
    ClassBlock* cb = CLASS_CB(class);
    obj = (O)sysMalloc(ObjHeaderSize+ size*el_size);
    //------------------------
    obj->isArray = TRUE;
    obj->length = size;
    obj->atype = atype;
    //------------------------
    obj->class = class;
    obj->data = (unsigned int*)(obj+1);
    memset(obj+1, 0, size * el_size);

    obj->cb = cb;

    /*NOTE: this is used when visited the array*/
    obj->el_size = el_size;
    obj->copy_size = ObjHeaderSize+size*el_size;
    return obj;
}

/*
 * The opcode newarray has a atype, according to the type,
 * determining the el_size.
 * invoke by:OPC_NEWARRAY, OPC_ANEWARRAY
 */
O allocTypeArray(int type, int size, char* element_name)
{
    int el_size;
    C class;

    switch (type)
    {
    case T_BYTE:
        //class = primClass[0];
        class = loadClass("[B");
        el_size = 1;
        break;
    case T_CHAR:
        //class = primClass[4];
        class = loadClass("[C");
        el_size = 2;
        break;
    case T_SHORT:
        //class = primClass[1];
        class = loadClass("[S");
        el_size = 2;
        break;
    case T_INT:
        //class = primClass[2];
        class = loadClass("[I");
        el_size = 4;
        break;
    case T_BOOLEAN:
        //class = primClass[7];
        class = loadClass("[Z");
        el_size = 4;
        break;
    case T_FLOAT:
        //class = primClass[5];
        class = loadClass("F");
        el_size = 4;
        break;
    case T_DOUBLE:
        //class = primClass[6];
        class = loadClass("[D");
        el_size = 8;
        break;
    case T_LONG:
        //class = primClass[3];
        class = loadClass("[J");
        el_size = 8;
        break;
    case T_REFERENCE:
    {
        class = loadClass(element_name);
        el_size = 4;
        break;
    }
    default:
        throwException("Invalid array type!");
    }

    return allocArray(class, size, el_size, type);
}

#undef C
