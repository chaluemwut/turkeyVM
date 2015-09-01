#include <string.h>
#include <stdlib.h>
#include "../classloader/class.h"
#include "../util/exception.h"
#include "../main/turkey.h"
#include "../lib/error.h"
#include "../lib/assert.h"

#define C Class_t
#define O Object_t

extern int vmsize;
extern C primClass[];

static const int OBJ_HEADER_SIZE = sizeof(struct O);

void *sysMalloc(int n)
{
    void *mem = malloc(n);
    if (mem == NULL)
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
    ClassBlock_t *cb;
    O obj;
    int obj_size;

    cb = CLASS_CB(class);
    obj_size = cb->obj_size;
    obj = (O) sysMalloc(OBJ_HEADER_SIZE + sizeof(int) * obj_size);
    //---------------------------------
    obj->type = OBJECT_OBJECT;
    obj->length = obj_size;
    obj->atype = 0;
    //--------------------------
    obj->class = class;
    obj->binding = NULL;
    obj->data = (unsigned int *) (obj + 1);
    memset(obj + 1, 0, sizeof(int) * obj_size);
    obj->cb = cb;
    obj->el_size = sizeof(int);
    //obj->copy_size = OBJ_HEADER_SIZE+ sizeof(int)*obj_size;
    return obj;

}

/*
 * @parm class      array's class
 * @parm size       array's length
 * @parm el_size    slot size, also is the elements size
 * @parm atype      element type
 * @see allocTypeArray
 * @qcliu 2015/03/24
 */
O allocArray(C class, int size, int el_size, int atype)
{
    O obj;
    ClassBlock_t *cb;

    cb = CLASS_CB(class);
    obj = (O) sysMalloc(OBJ_HEADER_SIZE + size * el_size);
    //------------------------
    obj->type = OBJECT_ARRAY;
    obj->length = size;
    obj->atype = atype;
    //------------------------
    obj->class = class;
    obj->data = (unsigned int *) (obj + 1);
    memset(obj + 1, 0, size * el_size);
    obj->cb = cb;
    /*NOTE: this is used when visited the array */
    obj->el_size = el_size;
    //obj->copy_size = OBJ_HEADER_SIZE+size*el_size;
    return obj;
}

int objectSize(O obj)
{
    Assert_ASSERT(obj);
    switch (obj->type) {
    case OBJECT_ARRAY:
    case OBJECT_OBJECT:
    case OBJECT_STRING:
        return OBJ_HEADER_SIZE + obj->length * obj->el_size;
    default:
        ERROR("impossible");
    }
    return 0;
}

/*
 * The opcode newarray has a atype, according to the type,
 * determining the el_size.
 * invoke by:OPC_NEWARRAY, OPC_ANEWARRAY
 */
O allocTypeArray(int type, int size, char *element_name)
{
    int el_size;
    C class;

    switch (type) {
    case T_BYTE:
        class = loadClass("[B");
        el_size = 1;
        break;
    case T_CHAR:
        class = loadClass("[C");
        el_size = 2;
        break;
    case T_SHORT:
        class = loadClass("[S");
        el_size = 2;
        break;
    case T_INT:
        class = loadClass("[I");
        el_size = 4;
        break;
    case T_BOOLEAN:
        class = loadClass("[Z");
        el_size = 4;
        break;
    case T_FLOAT:
        class = loadClass("F");
        el_size = 4;
        break;
    case T_DOUBLE:
        class = loadClass("[D");
        el_size = 8;
        break;
    case T_LONG:
        class = loadClass("[J");
        el_size = 8;
        break;
    case T_REFERENCE:{
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
