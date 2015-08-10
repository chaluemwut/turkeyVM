/*------------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  interp.c                               */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2015/01/23                             */
/*  DESCRIPTION           :  this is interpreter for turkeyVM       */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 * 2015/01/25
 * Packing the stack operation by macros. So it's easy to use
 *
 *//*}}}*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "execute.h"
#include "../heapManager/alloc.h"
#include "../native/cast.h"
#include "../classloader/class.h"
#include "../classloader/resolve.h"
#include "../control/control.h"
#include "../util/testvm.h"
#include "../util/exception.h"
#include "../main/turkey.h"
#include "../util/jstring.h"
#include "../native/reflect.h"
#include "../lib/list.h"
#include "stackmanager.h"
#include "../lib/error.h"
#include "../lib/mem.h"
#include "../lib/trace.h"
#include "../lib/assert.h"

#define C Class_t
#define O Object_t
#define JF JFrame_t

// to contral the pc move
#define PCMOVE(x) PCIncrease(x);
#define PCBACK(x) PCDecrease(x);
#define READ_INT(v, p) v=(p[0]<<24|p[1]<<16|p[2]<<8|p[3])
#define READ_IDX(v,p) v=(p[0]<<8)|p[1]
#define READ_SHORT(v,p) READ_IDX(v,p);
#define READ_BYTE(v,p) v=*p

//vm.h

static int testnum;

static void exe_OPC_NOP(JF f)
{
}

static void exe_OPC_ACONST_NULL(JF f)
{
    int value = 0;
    push(f,&value, TYPE_INT);
}

static void exe_OPC_ICONST_M1(JF f)
{
    int value = -1;
    push(f,&value, TYPE_INT);
}

static void exe_OPC_ICONST_0(JF f)
{
    int value = 0;
    push(f,&value, TYPE_INT);
}
static void exe_OPC_ICONST_1(JF f)
{
    int value = 1;
    push(f,&value, TYPE_INT);
}
static void exe_OPC_ICONST_2(JF f)
{
    int value = 2;
    push(f,&value, TYPE_INT);
}
static void exe_OPC_ICONST_3(JF f)
{
    int value = 3;
    push(f,&value, TYPE_INT);
}

static void exe_OPC_ICONST_4(JF f)
{
    int value = 4;
    push(f,&value, TYPE_INT);
}

static void exe_OPC_ICONST_5(JF f)
{
    int value = 5;
    push(f,&value, TYPE_INT);
}

static void exe_OPC_LCONST_0(JF f)
{
    long long value = 0;
    push(f,&value, TYPE_LONG);
}

static void exe_OPC_LCONST_1(JF f)
{
    long long value = 1;
    push(f,&value, TYPE_LONG);
}

static void exe_OPC_FCONST_0(JF f)
{
    float value = 0;
    push(f,&value, TYPE_FLOAT);
}

static void exe_OPC_FCONST_1(JF f)
{
    float value = 1;
    push(f,&value, TYPE_FLOAT);
}

static void exe_OPC_FCONST_2(JF f)
{
    float value = 2;
    push(f,&value, TYPE_FLOAT);
}

static void exe_OPC_DCONST_0(JF f)
{
    double value = 0;
    push(f,&value, TYPE_DOUBLE);
}

static void exe_OPC_DCONST_1(JF f)
{
    double value = 1;
    push(f,&value, TYPE_DOUBLE);
}

static void exe_OPC_BIPUSH(JF f)
{
    int value = 0;
    PCMOVE(1);
    READ_BYTE(value, getCurrentPC());
    push(f,&value, TYPE_INT);
}

static void exe_OPC_SIPUSH(JF f)
{
    //XXX
    short value0;
    int value;
    PCMOVE(1);
    READ_SHORT(value0, getCurrentPC());
    PCMOVE(1);
    value = (int)value0;
    push(f,&value, TYPE_INT);

}

static void exe_OPC_LDC(JF f)
{
    //TODO
    int index = 0;
    u4 cp_info;
    PCMOVE(1);
    READ_BYTE(index, getCurrentPC());
    cp_info = CP_INFO(getCurrentCP(), index);
    switch (CP_TYPE(getCurrentCP(), index))
    {
        /*NOTE: possible???*/
    case RESOLVED:
    {
        ERROR("impossible");
        push(f,&cp_info, TYPE_INT);
        break;
    }
    case CONSTANT_Integer:
    {
        int value;
        value = (int)cp_info;
        push(f,&value, TYPE_INT);
        break;
    }
    case CONSTANT_Float:
    {
        float value;
        /*NOTE: get the value from constants_pool as the type float
         *      instead of get as the type int then reverse int to float.
         * @qcliu 2015/03/16
         */
        value = *(float*)&getCurrentCP()->info[index];
        push(f,&value, TYPE_FLOAT);
        break;
    }
    case CONSTANT_String:
    {
        if (CP_TYPE(getCurrentCP(), cp_info) != CONSTANT_Utf8)
            throwException("not UTF8@ interp.c ldc");

        /*
         * New a Object, which Class is java/lang/String
         * NOTE: should assign the value. The string is
         * alread in the constans-pool.
         * @qcliu 2015/03/08
         */
        O obj;
        int offset;
        char* s = CP_UTF8(getCurrentCP(), cp_info);

        /**/
        obj = (O)createString(s);
        if (obj == NULL)
            throwException("LDC error");
        push(f,&obj, TYPE_REFERENCE);

        break;
    }
    case CONSTANT_Class:
    {
        C class = (C)resolveClass(getCurrentClass(), index);
        if (class != NULL)
        {
            O obj = class->class;
            push(f,&obj, TYPE_REFERENCE);
        }
        else
        {
            throwException("ldc CONSTANT_Class");
        }
        break;
    }
    default:
        throwException("LDC error");

    }
}

static void exe_OPC_LDC_W(JF f)
{
    int index = 0;
    u4 cp_info;
    PCMOVE(1);
    READ_IDX(index, getCurrentPC());
    PCMOVE(1);

    cp_info = CP_INFO(getCurrentCP(), index);
    switch (CP_TYPE(getCurrentCP(), index))
    {
        /*NOTE: possible???*/
    case RESOLVED:
    {
        push(f,&cp_info, TYPE_INT);
        break;
    }
    case CONSTANT_Integer:
    {
        int value;
        value = (int)cp_info;
        push(f,&value, TYPE_INT);
        break;
    }
    case CONSTANT_Float:
    {
        float value;
        /*NOTE: get the value from constants_pool as the type float
         *      instead of get as the type int then reverse int to float.
         * @qcliu 2015/03/16
         */
        value = *(float*)&getCurrentCP()->info[index];
        push(f,&value, TYPE_FLOAT);
        break;
    }
    case CONSTANT_String:
    {
        if (CP_TYPE(getCurrentCP(), cp_info) != CONSTANT_Utf8)
            throwException("not UTF8@ interp.c ldc_w");

        /*
         * New a Object, which Class is java/lang/String
         * NOTE: should assign the value. The string is
         * alread in the constans-pool.
         * @qcliu 2015/03/08
         */
        O obj;
        int offset;
        char* s = CP_UTF8(getCurrentCP(), cp_info);

        /**/
        obj = (O)createString(s);
        if (obj == NULL)
            throwException("LDC error");
        //printStringObject(obj);
        push(f,&obj, TYPE_REFERENCE);

        break;
    }
    case CONSTANT_Class:
    {
        C class = (C)resolveClass(getCurrentClass(), index);
        if (class != NULL)
        {
            O obj = class->class;
            push(f,&obj, TYPE_REFERENCE);
        }
        else
        {
            throwException("ldc CONSTANT_Class");
        }
        break;
    }
    default:
        throwException("LDC error");
    }

}
static void exe_OPC_LDC2_W(JF f)
{
    int index = 0;
    u4 cp_info;
    PCMOVE(1);
    READ_IDX(index, getCurrentPC());
    PCMOVE(1);


    switch (CP_TYPE(getCurrentCP(), index))
    {
    case RESOLVED:
    {
        //TODO
        break;
    }
    case CONSTANT_Long:
    {
        long long value;
        value = *(long long*)&getCurrentCP()->info[index];
        push(f,&value, TYPE_LONG);
        break;
    }
    case CONSTANT_Double:
    {
        double value;
        value = *(double*)&getCurrentCP()->info[index];
        push(f,&value, TYPE_DOUBLE);
        break;
    }
    default:
        throwException("LDC2_W error");
    }
}
static void exe_OPC_ILOAD(JF f)
{
    int value;
    int locals_idx = 0;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    load(&value, TYPE_INT, locals_idx);
    push(f,&value, TYPE_INT);

}
static void exe_OPC_LLOAD(JF f)
{
    int locals_idx = 0;
    long long value;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    load(&value, TYPE_LONG, locals_idx);
    /*NOTE: long use PUSH2*/
    push(f,&value, TYPE_LONG);

}
static void exe_OPC_FLOAD(JF f)
{
    int locals_idx = 0;
    float value;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    load(&value, TYPE_FLOAT, locals_idx);
    push(f,&value, TYPE_FLOAT);

}
static void exe_OPC_DLOAD(JF f)
{
    int locals_idx = 0;
    double value;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    load(&value, TYPE_DOUBLE, locals_idx);

    push(f,&value, TYPE_DOUBLE);
}
static void exe_OPC_ALOAD(JF f)
{

    O objref;
    int locals_idx = 0;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    load(&objref, TYPE_REFERENCE, locals_idx);
    push(f,&objref, TYPE_REFERENCE);
}

static void exe_OPC_ILOAD_0(JF f)
{

    int value;
    load(&value, TYPE_INT, 0);
    push(f,&value, TYPE_INT);
}

static void exe_OPC_ILOAD_1(JF f)
{

    int value;
    load(&value, TYPE_INT, 1);
    push(f,&value, TYPE_INT);
}

static void exe_OPC_ILOAD_2(JF f)
{

    int value;
    load(&value, TYPE_INT, 2);
    push(f,&value, TYPE_INT);
}

static void exe_OPC_ILOAD_3(JF f)
{

    int value;
    load(&value, TYPE_INT, 3);
    push(f,&value, TYPE_INT);
}

static void exe_OPC_LLOAD_0(JF f)
{

    long long value;
    load(&value, TYPE_LONG, 0);
    push(f,&value, TYPE_LONG);
}
static void exe_OPC_LLOAD_1(JF f)
{

    long long value;
    load(&value, TYPE_LONG, 1);
    push(f,&value, TYPE_LONG);
}

static void exe_OPC_LLOAD_2(JF f)
{

    long long value;
    load(&value, TYPE_LONG, 2);
    push(f,&value, TYPE_LONG);
}

static void exe_OPC_LLOAD_3(JF f)
{

    long long value;
    load(&value, TYPE_LONG, 3);
    push(f,&value, TYPE_LONG);
}

static void exe_OPC_FLOAD_0(JF f)
{
    float value;
    load(&value, TYPE_FLOAT, 0);
    push(f,&value, TYPE_FLOAT);
}

static void exe_OPC_FLOAD_1(JF f)
{
    float value;
    load(&value, TYPE_FLOAT, 1);
    push(f,&value, TYPE_FLOAT);
}

static void exe_OPC_FLOAD_2(JF f)
{

    float value;
    load(&value, TYPE_FLOAT, 2);
    push(f,&value, TYPE_FLOAT);
}

static void exe_OPC_FLOAD_3(JF f)
{

    float value;
    load(&value, TYPE_FLOAT, 3);
    push(f,&value, TYPE_FLOAT);
}

static void exe_OPC_DLOAD_0(JF f)
{

    double value;
    load(&value, TYPE_DOUBLE, 0);
    push(f,&value, TYPE_DOUBLE);
}

static void exe_OPC_DLOAD_1(JF f)
{

    double value;
    load(&value, TYPE_DOUBLE, 1);
    push(f,&value, TYPE_DOUBLE);
}

static void exe_OPC_DLOAD_2(JF f)
{

    double value;
    load(&value, TYPE_DOUBLE, 2);
    push(f,&value, TYPE_DOUBLE);
}

static void exe_OPC_DLOAD_3(JF f)
{

    double value;
    load(&value, TYPE_DOUBLE, 3);
    push(f,&value, TYPE_DOUBLE);
}

static void exe_OPC_ALOAD_0(JF f)
{

    O obj;
    load(&obj, TYPE_REFERENCE, 0);
    push(f,&obj, TYPE_REFERENCE);
}

static void exe_OPC_ALOAD_1(JF f)
{

    O obj;
    load(&obj, TYPE_REFERENCE, 1);
    push(f,&obj, TYPE_REFERENCE);
}

static void exe_OPC_ALOAD_2(JF f)
{

    O obj;
    load(&obj, TYPE_REFERENCE, 2);
    push(f,&obj, TYPE_REFERENCE);
}

static void exe_OPC_ALOAD_3(JF f)
{

    O obj;
    load(&obj, TYPE_REFERENCE, 3);
    push(f,&obj, TYPE_REFERENCE);
}

static void exe_OPC_IALOAD(JF f)
{
    int value, index;
    O obj;
    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);

    if (obj == NULL)
        throwException("NullPointerException");
    Assert_ASSERT(obj->type == TYPE_ARRAY);
    if (index < 0 || index >= obj->length)
        throwException("OutofArrayBound");

    //value = obj->data[index];
    value = ARRAY_DATA(obj, index, int);

    push(f,&value, TYPE_INT);

}

static void exe_OPC_LALOAD(JF f)
{
    int index;
    O obj;
    long long value;

    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);


    value = ARRAY_DATA(obj, index, long long);
    push(f,&value, TYPE_LONG);

}

static void exe_OPC_FALOAD(JF f)
{
    float value;
    int index;
    O obj;

    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);

    if (obj == NULL)
        throwException("NullPointerException");
    Assert_ASSERT(obj->type == TYPE_ARRAY);
    if (index < 0 || index >= obj->length)
        throwException("OutofArrayBound");

    value = ARRAY_DATA(obj, index, float);
    push(f,&value, TYPE_FLOAT);

}

static void exe_OPC_DALOAD(JF f)
{
    double value;
    int index;
    O obj;

    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);

    if (obj == NULL)
        throwException("NullPointerException");
    Assert_ASSERT(obj->type == TYPE_ARRAY);
    if (index < 0 || index >= obj->length)
        throwException("OutofArrayBound");

    value = ARRAY_DATA(obj, index, double);
    push(f,&value, TYPE_DOUBLE);

}
static void exe_OPC_AALOAD(JF f)
{
    int index;
    O arrayref;
    O value;

    pop(f,&index, TYPE_INT);
    pop(f,&arrayref, TYPE_REFERENCE);

    if (arrayref == NULL)
        throwException("NullPointerException");
    Assert_ASSERT(arrayref->type == TYPE_ARRAY);
    if (index < 0 || index >= arrayref->length)
        throwException("OutofArrayBound");

    C class = arrayref->class;
    ClassBlock* cb = CLASS_CB(class);

    //printObjectWrapper(arrayref);

    value = ARRAY_DATA(arrayref, index, O);
    push(f,&value, TYPE_REFERENCE);

}

static void exe_OPC_BALOAD(JF f)
{
    char value;
    int index;
    O obj;

    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);

    if (obj == NULL)
        throwException("NullPointerException");
    Assert_ASSERT(obj->type == TYPE_ARRAY);
    if (index < 0 || index >= obj->length)
        throwException("OutofArrayBound");

    value = ARRAY_DATA(obj, index, char);
    int _value = (int)value;
    push(f,&_value, TYPE_INT);

}

static void exe_OPC_CALOAD(JF f)
{
    int index;
    O obj;
    char value;

    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);

    if (obj == NULL)
        throwException("NullPointerException");
    Assert_ASSERT(obj->type == TYPE_ARRAY);
    if (index < 0 || index >= obj->length)
        throwException("OutofArrayBound");

    /*NOTE: The element_size is 2*/
    value = ARRAY_DATA(obj, index, u2);
    push(f,&value, TYPE_CHAR);

}

static void exe_OPC_SALOAD(JF f)
{
    int index;
    O obj;
    short value;

    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);

    if (obj == NULL)
        throwException("NullPointerException");
    Assert_ASSERT(obj->type == TYPE_ARRAY);
    if (index < 0 || index >= obj->length)
        throwException("OutofArrayBound");


    value = ARRAY_DATA(obj, index, short);
    int _value = (int)value;
    push(f,&_value, TYPE_INT);

}

static void exe_OPC_ISTORE(JF f)
{
    int value;
    int locals_idx = 0;
    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    pop(f,&value, TYPE_INT);
    store(&value, TYPE_INT, locals_idx);

}

static void exe_OPC_LSTORE(JF f)
{
    long long value;
    int locals_idx = 0;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    pop(f,&value, TYPE_LONG);

    /*NOTE: The long and double use STORE and LOAD as well*/
    store(&value, TYPE_LONG, locals_idx);

}

static void exe_OPC_FSTORE(JF f)
{
    float value;
    int locals_idx = 0;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    pop(f,&value, TYPE_FLOAT);
    store(&value, TYPE_FLOAT, locals_idx);

}

static void exe_OPC_DSTORE(JF f)
{
    double value;
    int locals_idx = 0;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    pop(f,&value, TYPE_DOUBLE);

    store(&value, TYPE_DOUBLE, locals_idx);

}

static void exe_OPC_ASTORE(JF f)
{
    int value = 0;
    O obj;
    PCMOVE(1);
    READ_BYTE(value, getCurrentPC());
    //obj = popObject();
    pop(f,&obj, TYPE_REFERENCE);
    store(&obj, TYPE_REFERENCE, value);

}

static void exe_OPC_ISTORE_0(JF f)
{

    int value;
    pop(f,&value, TYPE_INT);
    store(&value, TYPE_INT, 0);
}

static void exe_OPC_ISTORE_1(JF f)
{

    int value;
    pop(f,&value, TYPE_INT);
    store(&value, TYPE_INT, 1);
}

static void exe_OPC_ISTORE_2(JF f)
{

    int value;
    pop(f,&value, TYPE_INT);
    store(&value, TYPE_INT, 2);
}

static void exe_OPC_ISTORE_3(JF f)
{

    int value;
    pop(f,&value, TYPE_INT);
    store(&value, TYPE_INT, 3);
}

static void exe_OPC_LSTORE_0(JF f)
{

    long long value;
    pop(f,&value, TYPE_LONG);
    store(&value, TYPE_LONG, 0);
}

static void exe_OPC_LSTORE_1(JF f)
{

    long long value;
    pop(f,&value, TYPE_LONG);
    store(&value, TYPE_LONG, 1);
}

static void exe_OPC_LSTORE_2(JF f)
{

    long long value;
    pop(f,&value, TYPE_LONG);
    store(&value, TYPE_LONG, 2);
}

static void exe_OPC_LSTORE_3(JF f)
{

    long long value;
    pop(f,&value, TYPE_LONG);
    store(&value, TYPE_LONG , 3);
}

static void exe_OPC_FSTORE_0(JF f)
{

    float value;
    pop(f,&value, TYPE_FLOAT);
    store(&value, TYPE_FLOAT, 0);
}

static void exe_OPC_FSTORE_1(JF f)
{

    float value;
    pop(f,&value, TYPE_FLOAT);
    store(&value, TYPE_FLOAT, 1);
}

static void exe_OPC_FSTORE_2(JF f)
{

    float value;
    pop(f,&value, TYPE_FLOAT);
    store(&value, TYPE_FLOAT, 2);
}

static void exe_OPC_FSTORE_3(JF f)
{

    float value;
    pop(f,&value, TYPE_FLOAT);
    store(&value, TYPE_FLOAT, 3);
}

static void exe_OPC_DSTORE_0(JF f)
{

    double value;
    pop(f,&value, TYPE_DOUBLE);
}

static void exe_OPC_DSTORE_1(JF f)
{

    double value;
    pop(f,&value, TYPE_DOUBLE);
    store(&value, TYPE_DOUBLE, 1);
}

static void exe_OPC_DSTORE_2(JF f)
{

    double value;
    pop(f,&value, TYPE_DOUBLE);
    store(&value, TYPE_DOUBLE, 2);
}

static void exe_OPC_DSTORE_3(JF f)
{

    double value;
    pop(f,&value, TYPE_DOUBLE);
    store(&value, TYPE_DOUBLE, 3);
}


static void exe_OPC_ASTORE_0(JF f)
{

    O obj;
    pop(f,&obj, TYPE_REFERENCE);
    store(&obj, TYPE_REFERENCE, 0);
}

static void exe_OPC_ASTORE_1(JF f)
{

    O obj;
    pop(f,&obj, TYPE_REFERENCE);
    store(&obj, TYPE_REFERENCE, 1);
}

static void exe_OPC_ASTORE_2(JF f)
{

    O obj;
    pop(f,&obj, TYPE_REFERENCE);
    store(&obj, TYPE_REFERENCE, 2);
}

static void exe_OPC_ASTORE_3(JF f)
{

    O obj;
    pop(f,&obj, TYPE_REFERENCE);
    store(&obj, TYPE_REFERENCE, 3);
}

static void exe_OPC_IASTORE(JF f)
{
    O obj;
    int value, index;
    pop(f,&value, TYPE_INT);
    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);


    if (obj == NULL)
        throwException("NullPointerException");
    Assert_ASSERT(obj->type == TYPE_ARRAY);
    if (index < 0 || index >= obj->length)
        throwException("OutofArrayBound");

    //obj->data[index] = value;
    ARRAY_DATA(obj, index, int) = value;

}

static void exe_OPC_LASTORE(JF f)
{
    O obj;
    long long value;
    int index;


    pop(f,&value, TYPE_LONG);
    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);

    ARRAY_DATA(obj, index, long long) = value;

}

static void exe_OPC_FASTORE(JF f)
{
    O obj;
    int index;
    float value;

    pop(f,&value, TYPE_FLOAT);
    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);

    if (obj == NULL)
        throwException("NullPointerException");
    Assert_ASSERT(obj->type == TYPE_ARRAY);
    if (index < 0 || index >= obj->length)
        throwException("OutofArrayBound");

    /*NOTE: the type if float*/
    ARRAY_DATA(obj, index, float) = value;

}
static void exe_OPC_DASTORE(JF f)
{
    O obj;
    int index;
    double value;

    pop(f,&value, TYPE_DOUBLE);
    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);

    if (obj == NULL)
        throwException("NullPointerException");
    Assert_ASSERT(obj->type == TYPE_ARRAY);
    if (index < 0 || index >= obj->length)
        throwException("OutofArrayBound");

    ARRAY_DATA(obj, index, double) = value;

}

static void exe_OPC_AASTORE(JF f)
{
    O arrayref;
    O value;
    int index;

    pop(f,&value, TYPE_REFERENCE);
    pop(f,&index, TYPE_INT);
    pop(f,&arrayref, TYPE_REFERENCE);

    if (arrayref == NULL)
        throwException("NullPointerException");
    Assert_ASSERT(arrayref->type == TYPE_ARRAY);
    if (index < 0 || index >= arrayref->length )
        throwException("OutofArrayBount");

    ARRAY_DATA(arrayref, index, O) = value;

}

static void exe_OPC_BASTORE(JF f)
{
    int index, value;
    char _value;
    O obj;

    pop(f,&value, TYPE_INT);
    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);
    _value = (char)value;

    if (obj == NULL)
        throwException("NullPointerException");
    Assert_ASSERT(obj->type == TYPE_ARRAY);
    if (index < 0 || index >= obj->length)
        throwException("OutofArrayBound");

    ARRAY_DATA(obj, index, char) = _value;

}

static void exe_OPC_CASTORE(JF f)
{
    O objref;
    int index, value;
    char _value;

    pop(f,&value, TYPE_INT);
    pop(f,&index, TYPE_INT);
    pop(f,&objref, TYPE_REFERENCE);
    _value = (char)value;

    ARRAY_DATA(objref, index, u2) = _value;

}

static void exe_OPC_SASTORE(JF f)
{
    O obj;
    int index, value;

    pop(f,&value, TYPE_INT);
    pop(f,&index, TYPE_INT);
    pop(f,&obj, TYPE_REFERENCE);

    short _value = (short)value;
    ARRAY_DATA(obj, index, short) = _value;

}

static void exe_OPC_POP(JF f)
{

    int temp;
    pop(f,&temp, TYPE_INT);
}

static void exe_OPC_POP2(JF f)
{

    long long temp;
    pop(f,&temp, TYPE_LONG);
}

static void exe_OPC_DUP(JF f)
{
    int value;

    pop(f,&value, TYPE_INT);
    push(f,&value, TYPE_INT);
    push(f,&value, TYPE_INT);

    //memcpy(f->ostack+1,f->ostack, sizeof(int));
    //f->ostack++;

}

static void exe_OPC_DUP_X1(JF f)
{
    int value1, value2;
    pop(f,&value1, TYPE_INT);
    pop(f,&value2, TYPE_INT);

    push(f,&value1, TYPE_INT);
    push(f,&value2, TYPE_INT);
    push(f,&value1, TYPE_INT);

}

static void exe_OPC_DUP_X2(JF f)
{

}
static void exe_OPC_DUP2(JF f)
{
}
static void exe_OPC_DUP2_X1(JF f)
{
}
static void exe_OPC_DUP2_X2(JF f)
{
}

static void Trace_exe_OPC_SWAP(JF f)
{
    int value1;
    int value2;

    pop(f, &value1, TYPE_INT);
    pop(f, &value2, TYPE_INT);
    push(f, &value1, TYPE_INT);
    push(f, &value2, TYPE_INT);

}
static void exe_OPC_SWAP(JF f)
{
    Trace_Opc("opc_swap", Trace_exe_OPC_SWAP,(f), printStack, printStack);
}

static void exe_OPC_IADD(JF f)
{
    int value1, value2, result;

    pop(f,&value1, TYPE_INT);
    pop(f,&value2, TYPE_INT);
    result = value1 + value2;
    push(f,&result, TYPE_INT);

}

static void exe_OPC_LADD(JF f)
{
    long long value1, value2, result;
    pop(f,&value2, TYPE_LONG);
    pop(f,&value1, TYPE_LONG);
    result = value1 + value2;
    push(f,&result, TYPE_LONG);

}

static void exe_OPC_FADD(JF f)
{
    //TODO
    float value1, value2, result;
    pop(f,&value2, TYPE_FLOAT);
    pop(f,&value1, TYPE_FLOAT);
    result = value1 + value2;
    push(f,&result, TYPE_FLOAT);

}

static void exe_OPC_DADD(JF f)
{
    double value1, value2, result;
    pop(f,&value2, TYPE_DOUBLE);
    pop(f,&value1, TYPE_DOUBLE);
    result = value1 + value2;
    push(f,&result, TYPE_DOUBLE);

}

static void exe_OPC_ISUB(JF f)
{
    int value1, value2, result;
    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);
    result = value1 - value2;
    push(f,&result, TYPE_INT);

}

static void exe_OPC_LSUB(JF f)
{
    long long value1, value2, result;
    pop(f,&value2, TYPE_LONG);
    pop(f,&value1, TYPE_LONG);
    result = value1 - value2;
    push(f,&result, TYPE_LONG);

}

static void exe_OPC_FSUB(JF f)
{
    float value1, value2, result;
    pop(f,&value2, TYPE_FLOAT);
    pop(f,&value1, TYPE_FLOAT);
    result = value1 - value2;
    push(f,&result, TYPE_FLOAT);

}

static void exe_OPC_DSUB(JF f)
{
    double value1, value2, result;
    pop(f,&value2, TYPE_DOUBLE);
    pop(f,&value1, TYPE_DOUBLE);
    result = value1 - value2;
    push(f,&result, TYPE_DOUBLE);

}

static void exe_OPC_IMUL(JF f)
{
    int value1, value2, result;
    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);
    result = value1 * value2;
    push(f,&result, TYPE_INT);

}

static void exe_OPC_LMUL(JF f)
{
    long long value1, value2, result;
    pop(f,&value2, TYPE_LONG);
    pop(f,&value1, TYPE_LONG);
    result = value1 * value2;
    push(f,&result, TYPE_LONG);

}

static void exe_OPC_FMUL(JF f)
{
    float value1, value2, result;
    pop(f,&value2, TYPE_FLOAT);
    pop(f,&value1, TYPE_FLOAT);
    result = value1 * value2;
    push(f,&result, TYPE_FLOAT);

}

static void exe_OPC_DMUL(JF f)
{
    double value1, value2, result;

    pop(f,&value2, TYPE_DOUBLE);
    pop(f,&value1, TYPE_DOUBLE);

    result = value1 * value2;

    push(f,&result, TYPE_DOUBLE);

}

static void exe_OPC_IDIV(JF f)
{
    int value1, value2, result;
    pop(f,&value2, TYPE_INT);

    if(0 == value2)
        throwException("ArithmeticException");

    pop(f,&value1, TYPE_INT);
    result = value1/value2;
    push(f,&result, TYPE_INT);

}

static void exe_OPC_LDIV(JF f)
{
    long long value1, value2, result;
    pop(f,&value2, TYPE_LONG);

    if (0 == value2)
        throwException("ArithmeticException");

    pop(f,&value1,TYPE_LONG);
    result = value1 / value2;
    push(f,&result, TYPE_LONG);

}

static void exe_OPC_FDIV(JF f)
{
    float value1, value2, result;
    pop(f,&value2, TYPE_FLOAT);
    if (0 == value2)
        throwException("ArithmeticException");

    pop(f,&value1, TYPE_FLOAT);
    result = value1/value2;
    push(f,&result, TYPE_FLOAT);

}

static void exe_OPC_DDIV(JF f)
{
    double value1, value2, result;

    pop(f,&value2, TYPE_DOUBLE);
    if (0 == value2)
        throwException("ArithmeticException");
    pop(f,&value1, TYPE_DOUBLE);
    result = value1/value2;
    push(f,&result, TYPE_DOUBLE);

}

static void exe_OPC_IREM(JF f)
{
    /*NOTE: The JVM Specification define is
     *      value1-(value1/value2)*value2
     *@qcliu 2015/03/15
     */
    int value1, value2, result;
    pop(f,&value2, TYPE_INT);

    if (0 == value2)
        throwException("ArithmeticException");

    pop(f,&value1, TYPE_INT);
    result = value1 - (value1/value2)*value2;
    push(f,&result, TYPE_INT);

}

static void exe_OPC_LREM(JF f)
{
    long long value1, value2, result;
    pop(f,&value2, TYPE_LONG);

    if (0 == value2)
        throwException("ArithmeticException");

    pop(f,&value1, TYPE_LONG);
    result = value1 % value2;
    push(f,&result, TYPE_LONG);

}

static void exe_OPC_FREM(JF f)
{
    DEBUG("TODO");
    exit(0);
    /*
       float value1, value2, result;
       POP(value2, float);
       if (0 == value2)
       throwException("ArithmeticException");
       POP(value1, float);
       result = value1 -(value2*(value1/value2));
       PUSH(result, float);
       */

}

static void exe_OPC_DREM(JF f)
{
    DEBUG("TODO");
    exit(0);
    /*
       double value1, value2, result;

       POP2(value2, double);
       if (0 == value2)
       throwException("ArithmeticException");
       POP2(value1, double);
       result = value1 -(value2*(value1/value2));
       PUSH2(result, double);

    */
}

static void exe_OPC_INEG(JF f)
{
    int value, result;
    pop(f,&value, TYPE_INT);
    result = 0 - value;
    push(f,&result, TYPE_INT);

}

static void exe_OPC_LNEG(JF f)
{

    long long value, result;
    pop(f,&value, TYPE_LONG);
    result = 0 - value;
    push(f,&result, TYPE_LONG);
}

static void exe_OPC_FNEG(JF f)
{

    float value, result;

    pop(f,&value, TYPE_FLOAT);
    result =-value;
    push(f,&result, TYPE_FLOAT);
}

static void exe_OPC_DNEG(JF f)
{
    double value, result;

    pop(f,&value, TYPE_DOUBLE);
    result = -value;
    push(f,&result, TYPE_DOUBLE);

}

static void exe_OPC_ISHL(JF f)
{
    int value1, value2, result;

    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);
    value2 = value2 & 0x0000001f;
    result = value1 << value2;
    push(f,&result, TYPE_INT);

}

static void exe_OPC_LSHL(JF f)
{
    long long value1, result;
    int value2;

    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_LONG);
    value2 = value2 & 0x0000003f;
    result = value1 << value2;

    push(f,&result, TYPE_LONG);

}

static void exe_OPC_ISHR(JF f)
{
    int value1, value2, result;

    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);
    value2 = value2 & 0x0000001f;
    result = value1 >> value2;
    push(f,&result, TYPE_INT);

}

static void exe_OPC_LSHR(JF f)
{
    long long value1, result;
    int value2;

    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_LONG);
    value2 = value2 & 0x0000003f;
    result = value1 >> value2;

    push(f,&result, TYPE_LONG);

}

static void exe_OPC_IUSHR(JF f)
{
    int value2, result;
    unsigned int value1;

    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_UINT);

    value2 = value2 & 0x0000001f;
    result = value1 >> value2;
    push(f,&result, TYPE_INT);

}

static void exe_OPC_LUSHR(JF f)
{
    long long value1, result;
    int value2;

    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_ULONG);
    value2 = value2 & 0x0000003f;
    result = value1 >> value2;
    push(f,&result, TYPE_LONG);

}

static void exe_OPC_IAND(JF f)
{
    int value1, value2;

    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);
    value1 = value1 & value2;
    push(f,&value1, TYPE_INT);

}

static void exe_OPC_LAND(JF f)
{
    long long value1, value2;
    pop(f,&value2, TYPE_LONG);
    pop(f,&value1, TYPE_LONG);
    value1 = value1 & value2;
    push(f,&value1, TYPE_LONG);

}

static void exe_OPC_IOR(JF f)
{
    int value1, value2;

    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);
    value1 = value1|value2;
    push(f,&value1, TYPE_INT);

}

static void exe_OPC_LOR(JF f)
{
    long long value1, value2;

    pop(f,&value2, TYPE_LONG);
    pop(f,&value1, TYPE_LONG);
    value1 = value1 | value2;
    push(f,&value1, TYPE_LONG);

}
static void exe_OPC_IXOR(JF f)
{
    int value1, value2;

    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);
    value1 = value1 ^ value2;
    push(f,&value1, TYPE_INT);

}

static void exe_OPC_LXOR(JF f)
{
    long long value1, value2;

    pop(f,&value2, TYPE_LONG);
    pop(f,&value1, TYPE_LONG);
    value1 = value1 ^ value2;
    push(f,&value1, TYPE_LONG);

}

static void exe_OPC_IINC(JF f)
{
    u4 index;

    int constt, value;
    char c;
    PCMOVE(1);
    READ_BYTE(index, getCurrentPC());
    PCMOVE(1);

    /*NOTE: extend the byte constt to int*/
    constt = 0;
    READ_BYTE(c, getCurrentPC());
    constt = (int)c;

    load(&value, TYPE_INT, index);
    value += constt;
    store(&value, TYPE_INT, index);

}

static void Trace_exe_OPC_I2L(JF f)
{   
    int value;
    long long result;

    pop(f,&value, TYPE_INT);
    result = (long long)value;
    push(f,&result, TYPE_LONG);
}

static void exe_OPC_I2L(JF f)
{
    Trace_Opc("opc_i2l", Trace_exe_OPC_I2L,(f), printStack, printStack);
}

static void exe_OPC_I2F(JF f)
{
    int value;
    float result;

    pop(f,&value, TYPE_INT);
    result = (float)value;
    push(f,&result, TYPE_FLOAT);

}

static void exe_OPC_I2D(JF f)
{
    int value;
    double result;

    pop(f,&value, TYPE_INT);
    result = (double)value;
    push(f,&result, TYPE_DOUBLE);

}

static void exe_OPC_L2I(JF f)
{
    long long value;
    int result;

    pop(f,&value, TYPE_LONG);
    result = (int)value;
    push(f,&result, TYPE_INT);

}

static void exe_OPC_L2F(JF f)
{
    long long value;
    float result;

    pop(f,&value, TYPE_LONG);
    result = (float)value;
    push(f,&result, TYPE_FLOAT);

}

static void exe_OPC_L2D(JF f)
{
    long long value;
    double result;

    pop(f,&value, TYPE_LONG);
    result = (double)value;
    push(f,&result, TYPE_DOUBLE);

}

static void exe_OPC_F2I(JF f)
{
    float value;
    int result;

    pop(f,&value, TYPE_FLOAT);
    result = (int)value;
    push(f,&result, TYPE_INT);

}

static void exe_OPC_F2L(JF f)
{
    float value;
    long long result;

    pop(f,&value, TYPE_FLOAT);
    result = (long long)value;
    push(f,&result, TYPE_LONG);

}

static void exe_OPC_F2D(JF f)
{
    float value;
    double result;

    pop(f,&value, TYPE_FLOAT);
    result = (double)value;
    push(f,&result, TYPE_DOUBLE);

}

static void exe_OPC_D2I(JF f)
{
    double value;
    int result;

    pop(f,&value ,TYPE_DOUBLE);
    result = (int)value;
    push(f,&result, TYPE_INT);

}

static void exe_OPC_D2L(JF f)
{
    double value;
    long long result;

    pop(f,&value ,TYPE_DOUBLE);
    result = (long long)value;
    push(f,&result, TYPE_LONG);

}

static void exe_OPC_D2F(JF f)
{
    double value;
    float result;

    pop(f,&value, TYPE_DOUBLE);
    result = (float)value;
    push(f,&result, TYPE_FLOAT);

}

static void exe_OPC_I2B(JF f)
{

}
static void exe_OPC_I2C(JF f)
{
}
static void exe_OPC_I2S(JF f)
{
}

static void exe_OPC_LCMP(JF f)
{
    long long value1, value2;
    int result;

    pop(f,&value2, TYPE_LONG);
    pop(f,&value1, TYPE_LONG);
    if (value1 > value2)
    {
        result = 1;
    }
    else if (value1 == value2)
    {
        result = 0;
    }
    else
    {
        result = -1;
    }
    push(f,&result, TYPE_INT);

}

static void exe_OPC_FCMPL(JF f)
{
    float value1, value2;
    int result;

    pop(f,&value2, TYPE_FLOAT);
    pop(f,&value1, TYPE_FLOAT);
    if (value1 > value2)
    {
        result = 1;
    }
    else if (value1 == value2)
    {
        result = 0;
    }
    else
    {
        result = -1;
    }
    push(f,&result, TYPE_INT);

}
static void exe_OPC_FCMPG(JF f)
{
}
static void exe_OPC_DCMPL(JF f)
{
}
static void exe_OPC_DCMPG(JF f)
{
}

static void exe_OPC_IFEQ(JF f)
{
    int value;
    pop(f,&value, TYPE_INT);
    if (0 == value)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        PCMOVE(offset - 2);

    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IFNE(JF f)
{
    int value;
    pop(f,&value, TYPE_INT);
    if (0 != value)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IFLT(JF f)
{
    int value;
    pop(f,&value, TYPE_INT);
    if (value < 0)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IFGE(JF f)
{
    int value;
    pop(f,&value, TYPE_INT);
    if (value >= 0)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IFGT(JF f)
{
    int value;
    pop(f,&value, TYPE_INT);
    if (value > 0)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IFLE(JF f)
{
    int value;
    pop(f,&value, TYPE_INT);
    if (value <= 0)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IF_ICMPEQ(JF f)
{
    int value1, value2;

    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);

    if (value1 == value2)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());

        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IF_ICMPNE(JF f)
{
    int value1, value2;
    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);

    if (value1 != value2)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());

        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IF_ICMPLT(JF f)
{
    int value1, value2;
    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);
    if (value1 < value2)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        /*NOTE: the offset */
        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}
static void exe_OPC_IF_ICMPGE(JF f)
{
    int value1, value2;
    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);
    if (value1 >= value2)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        /*NOTE: the offset is in terms of OPC_IF_ICMPGE*/

        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IF_ICMPGT(JF f)
{
    int value1, value2;
    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);
    if (value1 > value2)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IF_ICMPLE(JF f)
{
    int value1, value2;
    pop(f,&value2, TYPE_INT);
    pop(f,&value1, TYPE_INT);
    if (value1 <= value2)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IF_ACMPEQ(JF f)
{
    O value1, value2;
    pop(f,&value2, TYPE_REFERENCE);
    pop(f,&value1, TYPE_REFERENCE);
    int r;
    if (value1->type == TYPE_STRING && 
                value2->type == TYPE_STRING)
    {
        r = Jstring_equals(value1, value2);
    }
    else
    {
        r = (value1==value2);
    }

    if (r)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IF_ACMPNE(JF f)
{
    O value1, value2;
    pop(f,&value2, TYPE_REFERENCE);
    pop(f,&value1, TYPE_REFERENCE);
    int r;
    if (value1->type == TYPE_STRING && 
                value2->type == TYPE_STRING)
    {
        r = !Jstring_equals(value1, value2);
    }
    else
    {
        r = (value1 != value2);
    }

    if (r)
    {
        short offset;
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        PCMOVE(offset-2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_GOTO(JF f)
{
    short offset;
    PCMOVE(1);
    READ_IDX(offset, getCurrentPC());
    /*
     * Since before READ_IDX has already pc+1, and
     * when break switch the there will pc+1 autolly
     * So, the real offset is offset - 2.
     */
    PCMOVE(offset - 2);

}

static void exe_OPC_JSR(JF f)
{
    short offset;
    int address;

    PCMOVE(1);
    READ_IDX(offset, getCurrentPC());
    PCMOVE(1);

    address = offset;
    push(f,&address, TYPE_INT);

}

static void exe_OPC_RET(JF f)
{

}

static void exe_OPC_TABLESWITCH(JF f)
{
    int def, low, high, index, base, pad, jump;

    base = getCurrentPCOffset();//record the location of opc_tableswitch
    int temp = getCurrentPCOffset();
    pad = 4-temp%4;//the padding bit(0~3)
    PCMOVE(pad);
    READ_INT(def, getCurrentPC());
    PCMOVE(4);
    READ_INT(low, getCurrentPC());
    PCMOVE(4);
    READ_INT(high, getCurrentPC());
    PCMOVE(4);

    pop(f,&index, TYPE_INT);
    /*default case*/
    if (index < low || index > high)
    {
        int temp = getCurrentPCOffset();
        int jump = def-(temp - base);
        PCMOVE(jump - 1);
        return;
    }

    /*other case*/

    /*
     * `index-low` determin where is the jump.Compiler can order
     * the case by lowest to highest automatically.
     *
     * `jump` is the offset relative to the current_pc.
     * But after READ_INT(jump, getCurrentPC()), the `jump`
     * is the offset relative to the OPC_TABLESWITCH which is
     * represented by `base`. So the pc is alread moved
     * pc_offset-base relative to base, and now it's still need
     * move jump-(pc_offset-base).
     * @qcliu 2015/03/16
     */
    PCMOVE((index-low)*4);
    READ_INT(jump, getCurrentPC());
    temp = getCurrentPCOffset();
    jump = jump - (temp - base);
    PCMOVE(jump - 1);

}

static void exe_OPC_LOOKUPSWITCH(JF f)
{
    int def;
    int npairs;
    int base;
    int temp;
    int pad;
    int key;
    int jump;

    pop(f,&key, TYPE_INT);
    base = getCurrentPCOffset();
    temp = getCurrentPCOffset();
    pad = 4-temp%4;
    PCMOVE(pad);
    READ_INT(def, getCurrentPC());
    PCMOVE(4);
    READ_INT(npairs, getCurrentPC());
    PCMOVE(4);

    int* match;
    int* _offset;
    Mem_newSize(match, npairs);
    Mem_newSize(_offset, npairs);
    int i;
    for (i=0; i<npairs; i++)
    {
        READ_INT(match[i], getCurrentPC());
        PCMOVE(4);
        READ_INT(_offset[i], getCurrentPC());
        PCMOVE(4);
    }

    /*
    printf("match:\n");
    for (i=0; i<npairs; i++)
      printf("%d\n", match[i]);
    printf("offset:\n");
    for (i=0; i<npairs; i++)
      printf("%d\n", _offset[i]);
      */

    //printf("key:%d\n", key);

    for (i=0; i<npairs; i++)
    {
        if (key != match[i])
          continue;

        temp = getCurrentPCOffset();
        jump = _offset[i]-(temp-base);
        PCMOVE(jump-1);
        return;

    }
    //default
    temp = getCurrentPCOffset();
    jump = def-(temp-base);
    PCMOVE(jump-1);
    return;

    

    ERROR("test");
}

static void exe_OPC_IRETURN(JF retFrame, JF f)
{
    //unsigned int* old_ostack = f->prev->ostack;
    int value;
    pop(f,&value, TYPE_INT);
    push(retFrame, &value, TYPE_INT);
    //returnValue(&value, TYPE_INT);

}
static void exe_OPC_LRETURN(JF f)
{
}
static void exe_OPC_FRETURN(JF f)
{
}

static void exe_OPC_DRETURN(JF retFrame, JF f)
{
    double value;
    pop(f,&value, TYPE_DOUBLE);

    push(retFrame, &value, TYPE_DOUBLE);
    //returnValue(&value, TYPE_DOUBLE);

}

static void exe_OPC_ARETURN(JF retFrame, JF f)
{
    O array_ref;
    pop(f,&array_ref, TYPE_REFERENCE);
    //returnValue(&array_ref, TYPE_REFERENCE);
    push(retFrame, &array_ref, TYPE_REFERENCE);

}
static void exe_OPC_RETURN(JF f)
{
}

static void exe_OPC_GETSTATIC(JF f)
{
    u2 fieldref_idx;
    FieldBlock* fb;
    PCMOVE(1);
    READ_IDX(fieldref_idx, getCurrentPC());
    PCMOVE(1);
    fb = resolveField(getCurrentClass(), fieldref_idx);

    Assert_ASSERT(fb);

    if (0 == strcmp(fb->type, "D"))
    {
        double value;
        value = *(double*)&(fb->static_value);
        push(f,&value, TYPE_DOUBLE);
    }
    else if (0 == strcmp(fb->type, "J"))
    {
        long long value;
        value = *(long long*)&(fb->static_value);
        push(f,&value, TYPE_LONG);
    }
    else if (0 == strcmp(fb->type, "F"))
    {
        float value;
        value = *(float*)&(fb->static_value);
        push(f,&value, TYPE_FLOAT);
    }
    else
    {
        int value;
        value = fb->static_value;
        push(f,&value, TYPE_INT);
    }

}

static void exe_OPC_PUTSTATIC(JF f)
{
    u2 fieldref_idx;
    FieldBlock* fb;
    PCMOVE(1);
    READ_IDX(fieldref_idx, getCurrentPC());
    PCMOVE(1);
    fb = resolveField(getCurrentClass(), fieldref_idx);
    Assert_ASSERT(fb);

    if (0 == strcmp(fb->type, "D"))
    {
        double value;
        pop(f,&value, TYPE_DOUBLE);
        *(double*)&(fb->static_value) = value;
    }
    else if (0 == strcmp(fb->type, "F"))
    {
        float value;
        pop(f,&value, TYPE_FLOAT);
        *(float*)&(fb->static_value) = value;
    }
    else if (0 == strcmp(fb->type, "J"))
    {
        long long value;
        pop(f,&value, TYPE_LONG);
        *(long long*)&(fb->static_value) = value;
    }
    else
    {
        int value;
        pop(f,&value, TYPE_INT);
        *(int*)&(fb->static_value) = value;
    }

}

static void exe_OPC_GETFIELD(JF f)
{
    O objref;
    int value, offset;
    O value2;
    u2 fieldref_idx;

    PCMOVE(1);
    READ_IDX(fieldref_idx, getCurrentPC());
    PCMOVE(1);

    FieldBlock* fb = resolveField(getCurrentClass(), fieldref_idx);


    if (fb == NULL)
        throwException("resolveField is NULL");
    //for test
    //if (0 == strcmp(fb->name, "buckets"))
    //printf("TEST: getfield buckets\n");

    offset = fb->offset;

    pop(f,&objref, TYPE_REFERENCE);
    if (0 == strcmp(fb->type, "J"))
    {
        long long value;
        value = OBJECT_DATA(objref, offset-1, long long);
        push(f,&value, TYPE_LONG);
    }
    else if (0 == strcmp(fb->type, "D"))
    {
        double value;
        value = OBJECT_DATA(objref, offset-1, double);
        push(f,&value, TYPE_DOUBLE);
    }
    else if (0 == strcmp(fb->type, "F"))
    {
        float value;
        value = OBJECT_DATA(objref, offset-1, float);
        push(f,&value, TYPE_FLOAT);
    }
    else
    {
        int value;
        value = OBJECT_DATA(objref, offset-1, int);
        push(f,&value, TYPE_INT);
    }

}

static void exe_OPC_PUTFIELD(JF f)
{
    O objref;
    int value, offset;
    C class;
    u2 fieldref_idx;
    FieldBlock* fb;


    PCMOVE(1);
    READ_IDX(fieldref_idx, getCurrentPC());
    PCMOVE(1);

    fb = resolveField(getCurrentClass(), fieldref_idx);
    if (fb == NULL)
        throwException("resolveField is NULL");

    //if (0 == strcmp(fb->name, "buckets"))
    //printf("TEST: putfield buckets\n");

    offset = fb->offset;//offest always need sub 1;
    if (0 == strcmp(fb->type, "J"))
    {
        long long value;
        pop(f,&value, TYPE_LONG);
        pop(f,&objref, TYPE_REFERENCE);
        //*(long long*)&objref->data[offset-1] = value;
        OBJECT_DATA(objref, offset-1, long long) = value;
    }
    else if (0 == strcmp(fb->type, "D"))
    {
        double value;
        pop(f,&value, TYPE_DOUBLE);
        pop(f,&objref, TYPE_REFERENCE);
        //*(double*)&objref->data[offset-1] = value;
        OBJECT_DATA(objref, offset-1, double) = value;
    }
    else if (0 == strcmp(fb->type, "F"))
    {
        float value;
        pop(f,&value, TYPE_FLOAT);
        pop(f,&objref, TYPE_REFERENCE);
        //*(float*)&objref->data[offset-1] = value;
        OBJECT_DATA(objref, offset-1, float) = value;
    }
    else
    {
        int value;
        pop(f,&value, TYPE_INT);
        pop(f,&objref, TYPE_REFERENCE);
        //*(int*)&objref->data[offset-1] = value;
        OBJECT_DATA(objref, offset-1, int) = value;

    }


    /*note: The offset need sub 1, and the Exception is
     *      throw in resolveField().
     */

}

static void exe_OPC_INVOKEVIRTUAL(JF f)
{
    u2 methodref_idx, name_type_idx, type_idx;
    MethodBlock* method;
    O objref;
    C class;
    u4 cp_info;
    char* type;
    int args_count;

    PCMOVE(1);
    READ_IDX(methodref_idx, getCurrentPC());
    PCMOVE(1);
    /* According to the Methodref_info, get the description of
     * the method. Then, get the args' count
     */
    cp_info = CP_INFO(getCurrentCP(), methodref_idx);

    /*
     * The methodref_idx maybe resovled.
     */
    //===============================================
    switch (CP_TYPE(getCurrentCP(), methodref_idx))
    {
    case RESOLVED:
    {
        method = (MethodBlock*)cp_info;
        break;
    }
    case CONSTANT_Methodref:
    {
        name_type_idx = cp_info >> 16;
        cp_info = CP_INFO(getCurrentCP(), name_type_idx);
        type_idx = cp_info>>16;
        type = CP_UTF8(getCurrentCP(), type_idx);

        /*
         * According to the args_count, find the objectref's location in
         * the stack.
         * @qcliu 2015/01/30
         */
        args_count = parseArgs(type);
        objref = *(O*)(f->ostack - args_count);
        class = objref->class;
        //char* cname = CLASSNAME(class);
        //if (0 == strcmp(cname, "java/io/BufferedInputStream"))
         // dumpObject(objref);

        method = (MethodBlock*)resolveMethod(class, methodref_idx);

        break;
    }
    default:
        throwException("invokevirtual error!\n");
    }
    //===============================================
    if (!method)
        throwException("NoSuchMethodError");

    executeMethod(method, NULL);

}

static void exe_OPC_INVOKESPECIAL(JF f)
{
    u2 methodref_idx, class_idx, name_type_idx, name_idx, type_idx;
    u4 cp_info;
    C class;
    C sym_class;
    ClassBlock* current_cb;
    MethodBlock* method;
    char *sym_classname, *name, *type;

    current_cb = CLASS_CB(getCurrentClass());
    PCMOVE(1);
    READ_IDX(methodref_idx, getCurrentPC());
    PCMOVE(1);
    cp_info = CP_INFO(getCurrentCP(), methodref_idx);

    /* The methodref_idx maybe resolved*/
    switch (CP_TYPE(getCurrentCP(), methodref_idx))
    {
    case RESOLVED:
    {
        method = (MethodBlock*)cp_info;
        break;
    }
    case CONSTANT_Methodref:
    {
        class_idx = cp_info;

        /*get the symbolic Class*/
        sym_class = (C)resolveClass(getCurrentClass(), class_idx);

        name_type_idx = cp_info >> 16;
        cp_info = CP_INFO(getCurrentCP(), name_type_idx);
        name_idx = cp_info;
        type_idx = cp_info>>16;

        name = CP_UTF8(getCurrentCP(), name_idx);
        type = CP_UTF8(getCurrentCP(), type_idx);

        /*
         * Determine weather the symbolic class or current class's superclass.
         */
        if ((strcmp(name, "<init>") !=0) && (current_cb->super == sym_class) &&
                (current_cb->access_flags & ACC_SUPER))
            class = current_cb->super;
        else
            class = sym_class;

        method = resolveMethod(class, methodref_idx);

        break;
    }
    default:
        throwException("invokespecial error!!!");
    }

    if (!method)
        throwException("NoSuchMethodError");

    executeMethod(method, NULL);

}

static void exe_OPC_INVOKESTATIC(JF f)
{
    u2 methodref_idx;
    u4 cp_info;
    u2 class_idx;
    char* classname;
    MethodBlock* method;

    PCMOVE(1);
    READ_IDX(methodref_idx, getCurrentPC());
    PCMOVE(1);
    cp_info = CP_INFO(getCurrentCP(), methodref_idx);

    /*
     * The methodref_idx maybe resolved.
     */
    switch (CP_TYPE(getCurrentCP(), methodref_idx))
    {
    case RESOLVED:
    {
        method = (MethodBlock*)cp_info;
        break;
    }
    case CONSTANT_Methodref:
    {
        /*high                    low
         *--------------------------
         *|name&type |class        |
         *--------------------------
         */
        class_idx = cp_info;
        //classname = CP_UTF8(getCurrentCP(), CP_INFO(getCurrentCP(), class_idx));

        /*
         * The class is the resovle_method class.It must be inited
         * before the method resovle.
         * note: in this case, can not use resolveClass. Because we
         *       don't know the obj Class's CONSTANT_Class belong to which Class
         */
        //C class = (C)loadClass(classname);
        C class = (C)resolveClass(getCurrentClass(), class_idx);
        method = (MethodBlock*)resolveMethod(class, methodref_idx);

        break;

    }
    default:
        throwException("invokestatic error!!!!");
    }

    if (method == NULL)
        throwException("invokestatic error!!!no such method");

    executeMethod(method, NULL);

}

static void exe_OPC_INVOKEINTERFACE(JF f)
{
    u2 methodref_idx, name_type_idx, type_idx;
    MethodBlock* method;
    O objref;
    C class;
    u4 cp_info;
    char* type;
    int args_count;

    PCMOVE(1);
    READ_IDX(methodref_idx,getCurrentPC());
    PCMOVE(1);

    PCMOVE(1);
    int count = 0;
    READ_BYTE(count, getCurrentPC());
    PCMOVE(1);
    int isZero = 0;
    READ_BYTE(isZero, getCurrentPC());
    PCMOVE(1);
    /* According to the Methodref_info, get the description of
     * the method. Then, get the args' count
     */
    cp_info = CP_INFO(getCurrentCP(), methodref_idx);

    /*
     * The methodref_idx maybe resovled.
     */
    //===============================================
    switch (CP_TYPE(getCurrentCP(), methodref_idx))
    {
    case RESOLVED:
    {
        throwException("invokeinterface resolved");
    }
    /*NOTE: this is InterfaceMethodref*/
    case CONSTANT_InterfaceMethodref:
    {
        name_type_idx = cp_info >> 16;
        cp_info = CP_INFO(getCurrentCP(), name_type_idx);
        type_idx = cp_info>>16;
        type = CP_UTF8(getCurrentCP(), type_idx);

        args_count = parseArgs(type);
        objref = *(O*)(f->ostack - args_count);
        class = objref->class;
        ClassBlock* cb = CLASS_CB(class);

        /*NOTE: resolveInterfaceMethod()*/
        method = (MethodBlock*)resolveInterfaceMethod(class, methodref_idx);

        break;
    }
    default:
        throwException("invokeInterface error!\n");
    }
    //===============================================
    if (!method)
        throwException("NoSuchMethodError");

    executeMethod(method, NULL);

}

static void exe_OPC_NEW(JF f)
{
    u2 class_idx;
    C class;
    O obj;

    PCMOVE(1);
    READ_IDX(class_idx, getCurrentPC());
    PCMOVE(1);
    class = resolveClass(getCurrentClass(), class_idx);
    obj = allocObject(class);
    push(f,&obj, TYPE_REFERENCE);

}

static void exe_OPC_NEWARRAY(JF f)
{
    int count, atype;
    O obj;
    PCMOVE(1);
    READ_BYTE(atype, getCurrentPC());
    pop(f,&count, TYPE_INT);

    obj = (O)allocTypeArray(atype, count, NULL);
    if (obj == NULL)
        throwException("newarray obj == NULL!");

    push(f,&obj, TYPE_REFERENCE);

}

static void exe_OPC_ANEWARRAY(JF f)
{
    int count;
    u2 index;
    O obj;
    C class;

    index = 0;
    PCMOVE(1);
    //READ_IDX(index, getCurrentPC());
    index=(getCurrentPC()[0]<<8)|(getCurrentPC()[1]);//test
    PCMOVE(1);

    /*need to resolveClass*/
    char* classname;
    getCurrentCP()->type[index];//test
    switch (CP_TYPE(getCurrentCP(), index))
    {
    case RESOLVED:
    {
        C class = (C)CP_INFO(getCurrentCP(), index);
        ClassBlock* cb = CLASS_CB(class);
        classname = cb->this_classname;
        break;
    }
    case CONSTANT_Class:
        classname = CP_UTF8(getCurrentCP(), CP_INFO(getCurrentCP(), index));
        break;
    default:
        throwException("anewarray error. not CONSTANS_CLASS");
    }

    /*NOTE: important!!!!!!
     * As referrence array, need reverse the name.The name
     * in constants_pool is only the element name.
     * @qcliu 2015/03/28
     */
    int len = strlen(classname);
    char ac_name[len + 4];

    if (classname[0] == '[')
        strcat(strcpy(ac_name, "["), classname);
    else
        strcat(strcat(strcpy(ac_name, "[L"), classname),";");


    pop(f,&count, TYPE_INT);
    obj = (O)allocTypeArray(T_REFERENCE, count, ac_name);
    if (obj == NULL)
        throwException("anewarray obj == NULL!");

    push(f,&obj, TYPE_REFERENCE);

}

static void exe_OPC_ARRAYLENGTH(JF f)
{
    O arrayref;
    int len;

    pop(f,&arrayref, TYPE_REFERENCE);

    C class = arrayref->class;
    ClassBlock* cb = CLASS_CB(class);

    if (0 == arrayref)
        throwException("NullPointerException");

    if (!arrayref->type)
        throwException("this obj is not a array.");

    len = arrayref->length;
    push(f,&len, TYPE_INT);

}

static void exe_OPC_ATHROW(JF f)
{
    ClassBlock* cb = CLASS_CB(getCurrentClass());
    printf("current_class:%s, current_method:%s,%s",
           cb->this_classname, getCurrentMethodName(), getCurrentMethodDesc());

    //for test
    //printList(head);
    throwException("athrow test");
    O obj;

    pop(f,&obj, TYPE_REFERENCE);
    //TODO
    /*do some work*/
    push(f,&obj, TYPE_REFERENCE);

}

static void exe_OPC_CHECKCAST(JF f)
{
    //TODO
    PCMOVE(1);
    PCMOVE(1);
}

static void exe_OPC_INSTANCEOF(JF f)
{
    //TODO
    O obj;
    u2 index;
    int result;

    PCMOVE(1);
    READ_IDX(index, getCurrentPC());
    PCMOVE(1);

    pop(f,&obj, TYPE_REFERENCE);

    if (obj == NULL)
    {
        result = 0;
    }
    else
    {
        C class = resolveClass(getCurrentClass(), index);
        if (class == NULL)
            throwException("instanceof error");

        //result = instanceOf(obj, class);
        result = isInstanceOf(class, obj->class);
    }


    push(f,&result, TYPE_INT);

}

static void exe_OPC_MONITORENTER(JF f)
{
    //TODO
    /*only for test*/
    O obj;
    pop(f,&obj, TYPE_REFERENCE);

}

static void exe_OPC_MONITOREXIT(JF f)
{

    //TODO
    O obj;
    pop(f,&obj, TYPE_REFERENCE);
}
static void exe_OPC_WIDE(JF f)
{
}
static void exe_OPC_MULTIANEWARRAY(JF f)
{
}

static void exe_OPC_IFNULL(JF f)
{
    /*the same as OPC_IF_ICMPGE
    */
    O obj;
    short offset;

    pop(f,&obj, TYPE_REFERENCE);

    if (obj == NULL)
    {
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}

static void exe_OPC_IFNONNULL(JF f)
{
    /*the same as OPC_IF_ICMPGE
    */
    O obj;
    short offset;

    pop(f,&obj, TYPE_REFERENCE);

    if (obj != NULL)
    {
        PCMOVE(1);
        READ_IDX(offset, getCurrentPC());
        PCMOVE(offset - 2);
    }
    else
    {
        PCMOVE(2);
    }

}
static void exe_OPC_GOTO_W(JF f)
{
}
static void exe_OPC_JSR_W(JF f)
{
}

/*
 * invoke by: executeMethod(), executeStaticMethod() in execute.c
 */
int executeJava(JF retFrame, JF f)
{
    // When the stack frame is move to the top, exit.
    if (f->mb == NULL)
    {
        printf("\nframe is NULL\n");
        exitVM();
    }


    /* pc_offset is the location of the pc in the code.
     * NOTE: OPC_TABLESWITCH is use pc_offset. And pc_offset
     *      must move with the current_pc.
     *@qcliu 2015/03/16
     */
    unsigned int code_length = getCurrentCodeLen();
    //printf("%d\n", code_length);


    //unsigned char* pc = getCurrentPC();
    //unsigned short max_locals = current_mb->max_locals;
    //unsigned short max_stack = current_mb->max_stack;

    //unsigned int* min_stack_pointer = f->ostack;
    //unsigned int* max_stack_pointer = f->ostack + max_stack;
    //ClassBlock* cc = CLASS_CB(getCurrentClass());

    //Native doesn't have a return
    if (0 == code_length)
    {
        printf("code_length == 0    ");
        ClassBlock* cb = CLASS_CB(getCurrentClass());
        printf("method name:%s,type:%s, class:%s\n", getCurrentMethodName(),getCurrentMethodDesc(), cb->this_classname);

        exit(0);
        // popFrame();
        /*
         * After the popFrame(), must return executeJava().
         */
        return 0;
    }

    /*
     * note: pc_offset must move along with the current_pc. So use
     *       the PCMOVE() as much as possible.
     */
    while (getCurrentPCOffset() < code_length)
    {
        u4 c = *getCurrentPC();

        if (dis_testinfo)
        {
            printf("\nopcode: ---------%s\n", op_code[c]);//for test
            printf("pc_offset:------------%d\n", getCurrentPCOffset());
            printf("getCurrentPC():---------%d\n", (unsigned int)getCurrentPC());
            printStack(f);
        }
        switch (c)
        {
        case OPC_NOP://0
            break;
        case OPC_ACONST_NULL://1
            exe_OPC_ACONST_NULL(f);
            break;
        case OPC_ICONST_M1://2
            exe_OPC_ICONST_M1(f);
            break;
        case OPC_ICONST_0://3
            exe_OPC_ICONST_0(f);
            break;
        case OPC_ICONST_1://4
            exe_OPC_ICONST_1(f);
            break;
        case OPC_ICONST_2://5
            exe_OPC_ICONST_2(f);
            break;
        case OPC_ICONST_3://6
            exe_OPC_ICONST_3(f);
            break;
        case OPC_ICONST_4://7
            exe_OPC_ICONST_4(f);
            break;
        case OPC_ICONST_5://8
            exe_OPC_ICONST_5(f);
            break;
        case OPC_LCONST_0://9
            exe_OPC_LCONST_0(f);
            break;
        case OPC_LCONST_1: //10
            exe_OPC_LCONST_1(f);
            break;
        case OPC_FCONST_0: //11
            exe_OPC_FCONST_0(f);
            break;
        case OPC_FCONST_1: //12
            exe_OPC_FCONST_1(f);
            break;
        case OPC_FCONST_2: //13
            exe_OPC_FCONST_2(f);
            break;
        case OPC_DCONST_0://14
            exe_OPC_DCONST_0(f);
            break;
        case OPC_DCONST_1://15
            exe_OPC_DCONST_1(f);
            break;
        case OPC_BIPUSH://16
            exe_OPC_BIPUSH(f);
            break;
        case OPC_SIPUSH://17
            exe_OPC_SIPUSH(f);
            break;
        case OPC_LDC://18
            exe_OPC_LDC(f);
            break;
        case OPC_LDC_W://19
            exe_OPC_LDC_W(f);
            break;
        case OPC_LDC2_W://20
            exe_OPC_LDC2_W(f);
            break;

        case OPC_ILOAD://21
            exe_OPC_ILOAD(f);
            break;
        case OPC_LLOAD://22
            exe_OPC_LLOAD(f);
            break;
        case OPC_FLOAD://23
            exe_OPC_FLOAD(f);
            break;
        case OPC_DLOAD://24
            exe_OPC_DLOAD(f);
            break;
        case OPC_ALOAD://25
            exe_OPC_ALOAD(f);
            break;
        case OPC_ILOAD_0://26 NOTE: not test
            exe_OPC_ILOAD_0(f);
            break;
        case OPC_ILOAD_1://27
            exe_OPC_ILOAD_1(f);
            break;
        case OPC_ILOAD_2://28
            exe_OPC_ILOAD_2(f);
            break;
        case OPC_ILOAD_3://29
            exe_OPC_ILOAD_3(f);
            break;
        case OPC_LLOAD_0://30
            exe_OPC_LLOAD_0(f);
            break;
        case OPC_LLOAD_1://31
            exe_OPC_LLOAD_1(f);
            break;
        case OPC_LLOAD_2://32
            exe_OPC_LLOAD_2(f);
            break;
        case OPC_LLOAD_3://33
            exe_OPC_LLOAD_3(f);
            break;
        case OPC_FLOAD_0://34
            exe_OPC_FLOAD_0(f);
            break;
        case OPC_FLOAD_1://35
            exe_OPC_FLOAD_1(f);
            break;
        case OPC_FLOAD_2://36
            exe_OPC_FLOAD_2(f);
            break;
        case OPC_FLOAD_3://37
            exe_OPC_FLOAD_3(f);
            break;
        case OPC_DLOAD_0://38
            exe_OPC_DLOAD_0(f);
            break;
        case OPC_DLOAD_1://39
            exe_OPC_DLOAD_1(f);
            break;
        case OPC_DLOAD_2://40
            exe_OPC_DLOAD_2(f);
            break;
        case OPC_DLOAD_3://41
            exe_OPC_DLOAD_3(f);
            break;
        case OPC_ALOAD_0://42
            exe_OPC_ALOAD_0(f);
            break;
        case OPC_ALOAD_1://43
            exe_OPC_ALOAD_1(f);
            break;
        case OPC_ALOAD_2://44
            exe_OPC_ALOAD_2(f);
            break;
        case OPC_ALOAD_3://45
            exe_OPC_ALOAD_3(f);
            break;
        case OPC_IALOAD://46
            exe_OPC_IALOAD(f);
            break;
        case OPC_LALOAD://47
            exe_OPC_LALOAD(f);
            break;
        case OPC_FALOAD://48
            exe_OPC_FALOAD(f);
            break;
        case OPC_DALOAD://49
            exe_OPC_DALOAD(f);
            break;
        case OPC_AALOAD://50
            exe_OPC_AALOAD(f);
            break;
        case OPC_BALOAD://51
            exe_OPC_BALOAD(f);
            break;
        case OPC_CALOAD://52
            exe_OPC_CALOAD(f);
            break;
        case OPC_SALOAD://53
            exe_OPC_SALOAD(f);
            break;
        case OPC_ISTORE://54
            exe_OPC_ISTORE(f);
            break;
        case OPC_LSTORE://55
            exe_OPC_LSTORE(f);
            break;
        case OPC_FSTORE://56
            exe_OPC_FSTORE(f);
            break;
        case OPC_DSTORE://57
            exe_OPC_DSTORE(f);
            break;
        case OPC_ASTORE://58
            exe_OPC_ASTORE(f);
            break;
        case OPC_ISTORE_0://59
            exe_OPC_ISTORE_0(f);
            break;
        case OPC_ISTORE_1://60
            exe_OPC_ISTORE_1(f);
            break;
        case OPC_ISTORE_2://61
            exe_OPC_ISTORE_2(f);
            break;
        case OPC_ISTORE_3://62
            exe_OPC_ISTORE_3(f);
            break;
        case OPC_LSTORE_0://63
            exe_OPC_LSTORE_0(f);
            break;
        case OPC_LSTORE_1://64
            exe_OPC_LSTORE_1(f);
            break;
        case OPC_LSTORE_2://65
            exe_OPC_LSTORE_2(f);
            break;
        case OPC_LSTORE_3://66
            exe_OPC_LSTORE_3(f);
            break;
        case OPC_FSTORE_0://67
            exe_OPC_FSTORE_0(f);
            break;
        case OPC_FSTORE_1://68
            exe_OPC_FSTORE_1(f);
            break;
        case OPC_FSTORE_2://69
            exe_OPC_FSTORE_2(f);
            break;
        case OPC_FSTORE_3://70
            exe_OPC_FSTORE_3(f);
            break;
        case OPC_DSTORE_0://71
            exe_OPC_DSTORE_0(f);
            break;
        case OPC_DSTORE_1://72
            exe_OPC_DSTORE_1(f);
            break;
        case OPC_DSTORE_2://73
            exe_OPC_DSTORE_2(f);
            break;
        case OPC_DSTORE_3://74
            exe_OPC_DSTORE_3(f);
            break;
        case OPC_ASTORE_0://75
            exe_OPC_ASTORE_0(f);
            break;
        case OPC_ASTORE_1://76
            exe_OPC_ASTORE_1(f);
            break;
        case OPC_ASTORE_2://77
            exe_OPC_ASTORE_2(f);
            break;
        case OPC_ASTORE_3://78
            exe_OPC_ASTORE_3(f);
            break;
        case OPC_IASTORE://79
            exe_OPC_IASTORE(f);
            break;
        case OPC_LASTORE://80
            exe_OPC_LASTORE(f);
            break;
        case OPC_FASTORE://81
            exe_OPC_FASTORE(f);
            break;
        case OPC_DASTORE://82
            exe_OPC_DASTORE(f);
            break;
        case OPC_AASTORE://83
            exe_OPC_AASTORE(f);
            break;
        case OPC_BASTORE://84
            exe_OPC_BASTORE(f);
            break;
        case OPC_CASTORE://85
            exe_OPC_CASTORE(f);
            break;
        case OPC_SASTORE://86
            exe_OPC_SASTORE(f);
            break;
        case OPC_POP://87
            exe_OPC_POP(f);
            break;
        case OPC_POP2://88
            exe_OPC_POP2(f);
            break;
        case OPC_DUP://89
            exe_OPC_DUP(f);
            break;
        case OPC_DUP_X1://90
            exe_OPC_DUP_X1(f);
            break;
        case OPC_SWAP://95
            exe_OPC_SWAP(f);
            break;
        case OPC_IADD://96
            exe_OPC_IADD(f);
            break;
        case OPC_LADD://97
            exe_OPC_LADD(f);
            break;
        case OPC_FADD://98
            exe_OPC_FADD(f);
            break;
        case OPC_DADD://99
            exe_OPC_DADD(f);
            break;
        case OPC_ISUB://100
            exe_OPC_ISUB(f);
            break;
        case OPC_LSUB://101
            exe_OPC_LSUB(f);
            break;
        case OPC_FSUB://102
            exe_OPC_FSUB(f);
            break;
        case OPC_DSUB://103
            exe_OPC_DSUB(f);
            break;
        case OPC_IMUL://104
            exe_OPC_IMUL(f);
            break;
        case OPC_LMUL://105
            exe_OPC_LMUL(f);
            break;
        case OPC_FMUL://106
            exe_OPC_FMUL(f);
            break;
        case OPC_DMUL://107
            exe_OPC_DMUL(f);
            break;
        case OPC_IDIV://108
            exe_OPC_IDIV(f);
            break;
        case OPC_LDIV://109
            exe_OPC_LDIV(f);
            break;
        case OPC_FDIV://110
            exe_OPC_FDIV(f);
            break;
        case OPC_DDIV://111
            exe_OPC_DDIV(f);
            break;
        case OPC_IREM://112
            exe_OPC_IREM(f);
            break;
        case OPC_LREM://113
            exe_OPC_LREM(f);
            break;
        case OPC_FREM://114
            DEBUG("TODO");
            exit(0);
            exe_OPC_FREM(f);
            break;
        case OPC_DREM://115
            DEBUG("TODO");
            exit(0);
            exe_OPC_DREM(f);
            break;
        case OPC_INEG://116
            exe_OPC_INEG(f);
            break;
        case OPC_LNEG://117
            exe_OPC_LNEG(f);
            break;
        case OPC_FNEG://118
            exe_OPC_FNEG(f);
            break;
        case OPC_DNEG://119
            exe_OPC_DNEG(f);
            break;
        case OPC_ISHL://120
            exe_OPC_ISHL(f);
            break;
        case OPC_LSHL://121
            exe_OPC_LSHL(f);
            break;
        case OPC_ISHR://122
            exe_OPC_ISHR(f);
            break;
        case OPC_LSHR://123
            exe_OPC_LSHR(f);
            break;
        case OPC_IUSHR://124
            exe_OPC_IUSHR(f);
            break;
        case OPC_LUSHR://125
            exe_OPC_LUSHR(f);
            break;
        case OPC_IAND://126
            exe_OPC_IAND(f);
            break;
        case OPC_LAND://127
            exe_OPC_LAND(f);
            break;
        case OPC_IOR://128
            exe_OPC_IOR(f);
            break;
        case OPC_LOR://129
            exe_OPC_LOR(f);
            break;
        case OPC_IXOR://130
            exe_OPC_IXOR(f);
            break;
        case OPC_LXOR://131
            exe_OPC_LXOR(f);
            break;
        case OPC_IINC://132
            exe_OPC_IINC(f);
            break;
        case OPC_I2L://133
            exe_OPC_I2L(f);
            break;
        case OPC_I2F://134
            exe_OPC_I2F(f);
            break;
        case OPC_I2D://135
            exe_OPC_I2D(f);
            break;
        case OPC_L2I://136
            exe_OPC_L2I(f);
            break;
        case OPC_L2F://137
            exe_OPC_L2F(f);
            break;
        case OPC_L2D://138
            exe_OPC_L2D(f);
            break;
        case OPC_F2I://139
            exe_OPC_F2I(f);
            break;
        case OPC_F2L://140
            exe_OPC_F2L(f);
            break;
        case OPC_F2D://141
            exe_OPC_F2D(f);
            break;
        case OPC_D2I://142
            exe_OPC_D2I(f);
            break;
        case OPC_D2L://143
            exe_OPC_D2L(f);
            break;
        case OPC_D2F://144
            exe_OPC_D2F(f);
            break;
            /*
             *operate the byte as int
             */
        case OPC_I2B://145
            //int value;
            //char result;

            // POP(value, int);
            // result = (char)value;
            // PUSH(result, char);
            break;
        case OPC_I2C://146
            break;
        case OPC_I2S://147
            //int value;
            //short result;

            //POP(value, int);
            //result = (short)value;
            //PUSH(result, short);
            break;
        case OPC_LCMP://148
            exe_OPC_LCMP(f);
            break;
        case OPC_FCMPG://150
        case OPC_FCMPL://149
            exe_OPC_FCMPL(f);
            break;
        case OPC_IFEQ://153
            exe_OPC_IFEQ(f);
            break;
        case OPC_IFNE://154
            exe_OPC_IFNE(f);
            break;
        case OPC_IFLT://155
            exe_OPC_IFLT(f);
            break;
        case OPC_IFGE://156
            exe_OPC_IFGE(f);
            break;
        case OPC_IFGT://157
            exe_OPC_IFGT(f);
            break;
        case OPC_IFLE://158
            exe_OPC_IFLE(f);
            break;
        case OPC_IF_ICMPEQ://159
            exe_OPC_IF_ICMPEQ(f);
            break;
        case OPC_IF_ICMPNE://160
            exe_OPC_IF_ICMPNE(f);
            break;
        case OPC_IF_ICMPLT://161
            exe_OPC_IF_ICMPLT(f);
            break;
        case OPC_IF_ICMPGE://162
            exe_OPC_IF_ICMPGE(f);
            break;
        case OPC_IF_ICMPGT://163
            exe_OPC_IF_ICMPGT(f);
            break;
        case OPC_IF_ICMPLE://164
            exe_OPC_IF_ICMPLE(f);
            break;
        case OPC_IF_ACMPEQ://165
            exe_OPC_IF_ACMPEQ(f);
            break;
        case OPC_IF_ACMPNE://166
            exe_OPC_IF_ACMPNE(f);
            break;
        case OPC_GOTO://167
            exe_OPC_GOTO(f);
            break;
        case OPC_JSR://168
            exe_OPC_JSR(f);
            break;
        case OPC_TABLESWITCH://170
            exe_OPC_TABLESWITCH(f);
            break;
        case OPC_LOOKUPSWITCH:
            exe_OPC_LOOKUPSWITCH(f);
            break;
        case OPC_IRETURN://172
            exe_OPC_IRETURN(retFrame,f);
            return 0;
            break;
        case OPC_FRETURN://174
            throwException("todo");
            break;
        case OPC_DRETURN://175
            exe_OPC_DRETURN(retFrame,f);
            return 0;
            break;
        case OPC_ARETURN://176
            exe_OPC_ARETURN(retFrame, f);
            return 0;
            break;
        case OPC_RETURN://177
            //printf("---------------current  stack top:%d\n", *(int*)f->ostack);
            //popFrame();
            return 0;
            break;
        case OPC_GETSTATIC://178
            exe_OPC_GETSTATIC(f);
            break;
        case OPC_PUTSTATIC://179
            exe_OPC_PUTSTATIC(f);
            break;
        case OPC_GETFIELD://180
            exe_OPC_GETFIELD(f);
            break;
        case OPC_PUTFIELD://181
            exe_OPC_PUTFIELD(f);
            break;
        case OPC_INVOKEVIRTUAL://182
            exe_OPC_INVOKEVIRTUAL(f);
            break;
            /*
             * First, should determine the class is current class or
             * super class.
             */
        case OPC_INVOKESPECIAL://183
            exe_OPC_INVOKESPECIAL(f);
            break;
        case OPC_INVOKESTATIC://184
            exe_OPC_INVOKESTATIC(f);
            break;
        case OPC_INVOKEINTERFACE://185
            exe_OPC_INVOKEINTERFACE(f);
            break;
        case OPC_NEW://187
            exe_OPC_NEW(f);
            break;
        case OPC_NEWARRAY://188
            exe_OPC_NEWARRAY(f);
            break;
        case OPC_ANEWARRAY://189
            exe_OPC_ANEWARRAY(f);
            break;
        case OPC_ARRAYLENGTH://190
            exe_OPC_ARRAYLENGTH(f);
            break;
        case OPC_ATHROW://191
            exe_OPC_ATHROW(f);
            break;
        case OPC_CHECKCAST://192
            exe_OPC_CHECKCAST(f);
            break;
        case OPC_INSTANCEOF://193
            exe_OPC_INSTANCEOF(f);
            break;
        case OPC_MONITORENTER://194
            exe_OPC_MONITORENTER(f);
            break;
        case OPC_MONITOREXIT://195
            exe_OPC_MONITOREXIT(f);
            break;
        case OPC_IFNULL://198
            exe_OPC_IFNULL(f);
            break;
        case OPC_IFNONNULL://199
            exe_OPC_IFNONNULL(f);
            break;
        default:
            printStack(f);
            printf("\nwrong opcode!!%s\n", op_code[c]);
            exit(0);
        }
        PCMOVE(1);
    }
    return 0;
}

#undef C
#undef O
#undef JF
