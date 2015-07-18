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
#include "../util/control.h"
#include "../util/testvm.h"
#include "../util/exception.h"
#include "../main/vm.h"
#include "../util/string.h"
#include "../native/reflect.h"
#include "../lib/list.h"
#include "stackmanager.h"

#define C Class_t
// to contral the pc move
#define PCMOVE(x) PCIncrease(x);
#define PCBACK(x) PCDecrease(x);
#define READ_INT(v, p) v=(p[0]<<24|p[1]<<16|p[2]<<8|p[3])
#define READ_IDX(v,p) v=(p[0]<<8)|p[1]
#define READ_SHORT(v,p) READ_IDX(v,p);
#define READ_BYTE(v,p) v=*p

//vm.h

static int testnum;

static void exe_OPC_NOP() {
}

static void exe_OPC_ACONST_NULL() {
    int value = 0;
    push(&value, TYPE_INT);
}

static void exe_OPC_ICONST_M1() {
    int value = -1;
    push(&value, TYPE_INT);
}

static void exe_OPC_ICONST_0() {
    int value = 0;
    push(&value, TYPE_INT);
}
static void exe_OPC_ICONST_1() {
    int value = 1;
    push(&value, TYPE_INT);
}
static void exe_OPC_ICONST_2() {
    int value = 2;
    push(&value, TYPE_INT);
}
static void exe_OPC_ICONST_3() {
    int value = 3;
    push(&value, TYPE_INT);
}

static void exe_OPC_ICONST_4() {
    int value = 4;
    push(&value, TYPE_INT);
}

static void exe_OPC_ICONST_5() {
    int value = 5;
    push(&value, TYPE_INT);
}

static void exe_OPC_LCONST_0() {
    long long value = 0;
    push(&value, TYPE_LONG);
}

static void exe_OPC_LCONST_1() {
    long long value = 1;
    push(&value, TYPE_LONG);
}

static void exe_OPC_FCONST_0(){
    float value = 0;
    push(&value, TYPE_FLOAT);
}

static void exe_OPC_FCONST_1() {
    float value = 1;
    push(&value, TYPE_FLOAT);
}

static void exe_OPC_FCONST_2() {
    float value = 2;
    push(&value, TYPE_FLOAT);
}
static void exe_OPC_DCONST_0() {
    double value = 0;
    push(&value, TYPE_DOUBLE);
}

static void exe_OPC_DCONST_1() {
    double value = 1;
    push(&value, TYPE_DOUBLE);
}

static void exe_OPC_BIPUSH() {
    int value = 0;
    PCMOVE(1);
    READ_BYTE(value, getCurrentPC());
    push(&value, TYPE_INT);
}

static void exe_OPC_SIPUSH() {
    //XXX
    short value0;
    int value;
    PCMOVE(1);
    READ_SHORT(value0, getCurrentPC());
    PCMOVE(1);
    value = (int)value0;
    push(&value, TYPE_INT);

}

static void exe_OPC_LDC() {
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
                push(&cp_info, TYPE_INT);
                break;
            }
        case CONSTANT_Integer:
            {
                int value;
                value = (int)cp_info;
                push(&value, TYPE_INT);
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
                push(&value, TYPE_FLOAT);
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
                Object* obj;
                int offset;
                char* s = CP_UTF8(getCurrentCP(), cp_info);

                /**/
                obj = (Object*)createString(s);
                if (obj == NULL)
                  throwException("LDC error");
                push(&obj, TYPE_REFERENCE);

                break;
            }
        case CONSTANT_Class:
            {
                C class = (C)resolveClass(getCurrentClass(), index);
                if (class != NULL)
                {
                    Object* obj = class->class;
                    push(&obj, TYPE_REFERENCE);
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

static void exe_OPC_LDC_W() {
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
                push(&cp_info, TYPE_INT);
                break;
            }
        case CONSTANT_Integer:
            {
                int value;
                value = (int)cp_info;
                push(&value, TYPE_INT);
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
                push(&value, TYPE_FLOAT);
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
                Object* obj;
                int offset;
                char* s = CP_UTF8(getCurrentCP(), cp_info);

                /**/
                obj = (Object*)createString(s);
                if (obj == NULL)
                  throwException("LDC error");
                //printStringObject(obj);
                push(&obj, TYPE_REFERENCE);

                break;
            }
        case CONSTANT_Class:
            {
                C class = (C)resolveClass(getCurrentClass(), index);
                if (class != NULL) {
                    Object* obj = class->class;
                    push(&obj, TYPE_REFERENCE);
                }
                else {
                    throwException("ldc CONSTANT_Class");
                }
                break;
            }
        default:
            throwException("LDC error");
    }

}
static void exe_OPC_LDC2_W() {
    int index = 0;
    u4 cp_info;
    PCMOVE(1);
    READ_IDX(index, getCurrentPC());
    PCMOVE(1);


    switch (CP_TYPE(getCurrentCP(), index)) {
        case RESOLVED: {
                           //TODO
                           break;
                       }
        case CONSTANT_Long: {
                                long long value;
                                value = *(long long*)&getCurrentCP()->info[index];
                                push(&value, TYPE_LONG);
                                break;
                            }
        case CONSTANT_Double: {
                                  double value;
                                  value = *(double*)&getCurrentCP()->info[index];
                                  push(&value, TYPE_DOUBLE);
                                  break;
                              }
        default:
                              throwException("LDC2_W error");
    }
}
static void exe_OPC_ILOAD() {
    int value;
    int locals_idx = 0;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    load(&value, TYPE_INT, locals_idx);
    push(&value, TYPE_INT);

}
static void exe_OPC_LLOAD() {
    int locals_idx = 0;
    long long value;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    load(&value, TYPE_LONG, locals_idx);
    /*NOTE: long use PUSH2*/
    push(&value, TYPE_LONG);

}
static void exe_OPC_FLOAD() {
    int locals_idx = 0;
    float value;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    load(&value, TYPE_FLOAT, locals_idx);
    push(&value, TYPE_FLOAT);

}
static void exe_OPC_DLOAD() {
    int locals_idx = 0;
    double value;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    load(&value, TYPE_DOUBLE, locals_idx);

    push(&value, TYPE_DOUBLE);
}
static void exe_OPC_ALOAD() {

    Object* objref;
    int locals_idx = 0;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    load(&objref, TYPE_REFERENCE, locals_idx);
    push(&objref, TYPE_REFERENCE);
}

static void exe_OPC_ILOAD_0() {

    int value;
    load(&value, TYPE_INT, 0);
    push(&value, TYPE_INT);
}

static void exe_OPC_ILOAD_1() {

    int value;
    load(&value, TYPE_INT, 1);
    push(&value, TYPE_INT);
}

static void exe_OPC_ILOAD_2() {

    int value;
    load(&value, TYPE_INT, 2);
    push(&value, TYPE_INT);
}

static void exe_OPC_ILOAD_3() {

    int value;
    load(&value, TYPE_INT, 3);
    push(&value, TYPE_INT);
}

static void exe_OPC_LLOAD_0() {

    long long value;
    load(&value, TYPE_LONG, 0);
    push(&value, TYPE_LONG);
}
static void exe_OPC_LLOAD_1() {

    long long value;
    load(&value, TYPE_LONG, 1);
    push(&value, TYPE_LONG);
}

static void exe_OPC_LLOAD_2() {

    long long value;
    load(&value, TYPE_LONG, 2);
    push(&value, TYPE_LONG);
}

static void exe_OPC_LLOAD_3() {

    long long value;
    load(&value, TYPE_LONG, 3);
    push(&value, TYPE_LONG);
}

static void exe_OPC_FLOAD_0() {
    float value;
    load(&value, TYPE_FLOAT, 0);
    push(&value, TYPE_FLOAT);
}

static void exe_OPC_FLOAD_1() {
    float value;
    load(&value, TYPE_FLOAT, 1);
    push(&value, TYPE_FLOAT);
}

static void exe_OPC_FLOAD_2() {

    float value;
    load(&value, TYPE_FLOAT, 2);
    push(&value, TYPE_FLOAT);
}

static void exe_OPC_FLOAD_3() {

    float value;
    load(&value, TYPE_FLOAT, 3);
    push(&value, TYPE_FLOAT);
}

static void exe_OPC_DLOAD_0() {

    double value;
    load(&value, TYPE_DOUBLE, 0);
    push(&value, TYPE_DOUBLE);
}

static void exe_OPC_DLOAD_1() {

    double value;
    load(&value, TYPE_DOUBLE, 1);
    push(&value, TYPE_DOUBLE);
}

static void exe_OPC_DLOAD_2() {

    double value;
    load(&value, TYPE_DOUBLE, 2);
    push(&value, TYPE_DOUBLE);
}

static void exe_OPC_DLOAD_3() {

    double value;
    load(&value, TYPE_DOUBLE, 3);
    push(&value, TYPE_DOUBLE);
}

static void exe_OPC_ALOAD_0() {

    Object* obj;
    load(&obj, TYPE_REFERENCE, 0);
    push(&obj, TYPE_REFERENCE);
}

static void exe_OPC_ALOAD_1() {

    Object* obj;
    load(&obj, TYPE_REFERENCE, 1);
    push(&obj, TYPE_REFERENCE);
}

static void exe_OPC_ALOAD_2() {

    Object* obj;
    load(&obj, TYPE_REFERENCE, 2);
    push(&obj, TYPE_REFERENCE);
}

static void exe_OPC_ALOAD_3() {

    Object* obj;
    load(&obj, TYPE_REFERENCE, 3);
    push(&obj, TYPE_REFERENCE);
}

static void exe_OPC_IALOAD() {
    int value, index;
    Object* obj;
    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);

    if (obj == NULL)
      throwException("NullPointerException");
    if (!obj->isArray)
      throwException("NoArray");
    if (index < 0 || index >= obj->length)
      throwException("OutofArrayBound");

    //value = obj->data[index];
    value = ARRAY_DATA(obj, index, int);

    push(&value, TYPE_INT);

}

static void exe_OPC_LALOAD() {
    int index;
    Object* obj;
    long long value;

    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);


    value = ARRAY_DATA(obj, index, long long);
    push(&value, TYPE_LONG);

}

static void exe_OPC_FALOAD() {
    float value;
    int index;
    Object* obj;

    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);

    if (obj == NULL)
      throwException("NullPointerException");
    if (!obj->isArray)
      throwException("NoArray");
    if (index < 0 || index >= obj->length)
      throwException("OutofArrayBound");

    value = ARRAY_DATA(obj, index, float);
    push(&value, TYPE_FLOAT);

}

static void exe_OPC_DALOAD() {
    double value;
    int index;
    Object* obj;

    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);

    if (obj == NULL)
      throwException("NullPointerException");
    if (!obj->isArray)
      throwException("obj is not array");
    if (index < 0 || index >= obj->length)
      throwException("OutofArrayBound");

    value = ARRAY_DATA(obj, index, double);
    push(&value, TYPE_DOUBLE);

}
static void exe_OPC_AALOAD() {
    int index;
    Object* arrayref;
    Object* value;

    pop(&index, TYPE_INT);
    pop(&arrayref, TYPE_REFERENCE);

    if (arrayref == NULL)
      throwException("NullPointerException");
    if (!arrayref->isArray)
      throwException("obj is not array");
    if (index < 0 || index >= arrayref->length)
      throwException("OutofArrayBound");

    C class = arrayref->class;
    ClassBlock* cb = CLASS_CB(class);

    //printObjectWrapper(arrayref);

    value = ARRAY_DATA(arrayref, index, Object*);
    push(&value, TYPE_REFERENCE);

}

static void exe_OPC_BALOAD() {
    char value;
    int index;
    Object* obj;

    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);

    if (obj == NULL)
      throwException("NullPointerException");
    if (!obj->isArray)
      throwException("obj is not array");
    if (index < 0 || index >= obj->length)
      throwException("OutofArrayBound");

    value = ARRAY_DATA(obj, index, char);
    int _value = (int)value;
    push(&value, TYPE_INT);

}

static void exe_OPC_CALOAD() {
    int index;
    Object* obj;
    char value;

    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);

    if (obj == NULL)
      throwException("NullPointerException");
    if (!obj->isArray)
      throwException("obj is not array");
    if (index < 0 || index >= obj->length)
      throwException("OutofArrayBound");

    /*NOTE: The element_size is 2*/
    value = ARRAY_DATA(obj, index, u2);
    push(&value, TYPE_CHAR);

}

static void exe_OPC_SALOAD() {
    int index;
    Object* obj;
    short value;

    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);

    if (obj == NULL)
      throwException("NullPointerException");
    if (!obj->isArray)
      throwException("obj is not array");
    if (index < 0 || index >= obj->length)
      throwException("OutofArrayBound");


    value = ARRAY_DATA(obj, index, short);
    int _value = (int)value;
    push(&_value, TYPE_INT);

}

static void exe_OPC_ISTORE() {
    int value;
    int locals_idx = 0;
    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    pop(&value, TYPE_INT);
    store(&value, TYPE_INT, locals_idx);

}

static void exe_OPC_LSTORE() {
    long long value;
    int locals_idx = 0;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    pop(&value, TYPE_LONG);

    /*NOTE: The long and double use STORE and LOAD as well*/
    store(&value, TYPE_LONG, locals_idx);


}

static void exe_OPC_FSTORE() {
    float value;
    int locals_idx = 0;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    pop(&value, TYPE_FLOAT);
    store(&value, TYPE_FLOAT, locals_idx);

}

static void exe_OPC_DSTORE() {
    double value;
    int locals_idx = 0;

    PCMOVE(1);
    READ_BYTE(locals_idx, getCurrentPC());
    pop(&value, TYPE_DOUBLE);

    store(&value, TYPE_DOUBLE, locals_idx);

}

static void exe_OPC_ASTORE() {
    int value = 0;
    Object* obj;
    PCMOVE(1);
    READ_BYTE(value, getCurrentPC());
    //obj = popObject();
    pop(&obj, TYPE_REFERENCE);
    store(&obj, TYPE_REFERENCE, value);

}

static void exe_OPC_ISTORE_0() {

    int value;
    pop(&value, TYPE_INT);
    store(&value, TYPE_INT, 0);
}

static void exe_OPC_ISTORE_1() {

    int value;
    pop(&value, TYPE_INT);
    store(&value, TYPE_INT, 1);
}

static void exe_OPC_ISTORE_2() {

    int value;
    pop(&value, TYPE_INT);
    store(&value, TYPE_INT, 2);
}

static void exe_OPC_ISTORE_3() {

    int value;
    pop(&value, TYPE_INT);
    store(&value, TYPE_INT, 3);
}

static void exe_OPC_LSTORE_0() {

    long long value;
    pop(&value, TYPE_LONG);
    store(&value, TYPE_LONG, 0);
}

static void exe_OPC_LSTORE_1() {

    long long value;
    pop(&value, TYPE_LONG);
    store(&value, TYPE_LONG, 1);
}

static void exe_OPC_LSTORE_2() {

    long long value;
    pop(&value, TYPE_LONG);
    store(&value, TYPE_LONG, 2);
}

static void exe_OPC_LSTORE_3() {

    long long value;
    pop(&value, TYPE_LONG);
    store(&value, TYPE_LONG , 3);
}

static void exe_OPC_FSTORE_0() {

    float value;
    pop(&value, TYPE_FLOAT);
    store(&value, TYPE_FLOAT, 0);
}

static void exe_OPC_FSTORE_1() {

    float value;
    pop(&value, TYPE_FLOAT);
    store(&value, TYPE_FLOAT, 1);
}

static void exe_OPC_FSTORE_2() {

    float value;
    pop(&value, TYPE_FLOAT);
    store(&value, TYPE_FLOAT, 2);
}

static void exe_OPC_FSTORE_3() {

    float value;
    pop(&value, TYPE_FLOAT);
    store(&value, TYPE_FLOAT, 3);
}

static void exe_OPC_DSTORE_0() {

    double value;
    pop(&value, TYPE_DOUBLE);
}

static void exe_OPC_DSTORE_1() {

    double value;
    pop(&value, TYPE_DOUBLE);
    store(&value, TYPE_DOUBLE, 1);
}

static void exe_OPC_DSTORE_2() {

    double value;
    pop(&value, TYPE_DOUBLE);
    store(&value, TYPE_DOUBLE, 2);
}

static void exe_OPC_DSTORE_3() {

    double value;
    pop(&value, TYPE_DOUBLE);
    store(&value, TYPE_DOUBLE, 3);
}


static void exe_OPC_ASTORE_0() {

    Object* obj;
    pop(&obj, TYPE_REFERENCE);
    store(&obj, TYPE_REFERENCE, 0);
}

static void exe_OPC_ASTORE_1() {

    Object* obj;
    pop(&obj, TYPE_REFERENCE);
    store(&obj, TYPE_REFERENCE, 1);
}

static void exe_OPC_ASTORE_2() {

    Object* obj;
    pop(&obj, TYPE_REFERENCE);
    store(&obj, TYPE_REFERENCE, 2);
}

static void exe_OPC_ASTORE_3() {

    Object* obj;
    pop(&obj, TYPE_REFERENCE);
    store(&obj, TYPE_REFERENCE, 3);
}

static void exe_OPC_IASTORE() {
    Object* obj;
    int value, index;
    pop(&value, TYPE_INT);
    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);


    if (obj == NULL)
      throwException("NullPointerException");

    if (!obj->isArray)
      throwException("NoArray");

    if (index < 0 || index >= obj->length)
      throwException("OutofArrayBound");

    //obj->data[index] = value;
    ARRAY_DATA(obj, index, int) = value;

}

static void exe_OPC_LASTORE() {
    Object* obj;
    long long value;
    int index;


    pop(&value, TYPE_LONG);
    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);

    ARRAY_DATA(obj, index, long long) = value;

}

static void exe_OPC_FASTORE() {
    Object* obj;
    int index;
    float value;

    pop(&value, TYPE_FLOAT);
    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);

    if (obj == NULL)
      throwException("NullPointerException");

    if (!obj->isArray)
      throwException("obj is not array");

    if (index < 0 || index >= obj->length)
      throwException("OutofArrayBound");

    /*NOTE: the type if float*/
    ARRAY_DATA(obj, index, float) = value;

}
static void exe_OPC_DASTORE() {
    Object* obj;
    int index;
    double value;

    pop(&value, TYPE_DOUBLE);
    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);

    if (obj == NULL)
      throwException("NullPointerException");
    if (!obj->isArray)
      throwException("obj is not array");
    if (index < 0 || index >= obj->length)
      throwException("OutofArrayBound");

    ARRAY_DATA(obj, index, double) = value;

}

static void exe_OPC_AASTORE() {
    Object* arrayref;
    Object* value;
    int index;

    pop(&value, TYPE_REFERENCE);
    pop(&index, TYPE_INT);
    pop(&arrayref, TYPE_REFERENCE);

    if (arrayref == NULL)
      throwException("NullPointerException");
    if (!arrayref->isArray)
      throwException("obj is not array");
    if (index < 0 || index >= arrayref->length )
      throwException("OutofArrayBount");

    ARRAY_DATA(arrayref, index, Object*) = value;

}

static void exe_OPC_BASTORE() {
    int index, value;
    char _value;
    Object* obj;

    pop(&value, TYPE_INT);
    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);
    _value = (char)value;

    if (obj == NULL)
      throwException("NullPointerException");
    if (!obj->isArray)
      throwException("obj is not array");
    if (index < 0 || index >= obj->length)
      throwException("OutofArrayBound");

    ARRAY_DATA(obj, index, char) = _value;

}

static void exe_OPC_CASTORE() {
    Object* objref;
    int index, value;
    char _value;

    pop(&value, TYPE_INT);
    pop(&index, TYPE_INT);
    pop(&objref, TYPE_REFERENCE);
    _value = (char)value;

    ARRAY_DATA(objref, index, u2) = _value;

}

static void exe_OPC_SASTORE() {
    Object* obj;
    int index, value;

    pop(&value, TYPE_INT);
    pop(&index, TYPE_INT);
    pop(&obj, TYPE_REFERENCE);

    short _value = (short)value;
    ARRAY_DATA(obj, index, short) = _value;

}

static void exe_OPC_POP() {

    int temp;
    pop(&temp, TYPE_INT);
}

static void exe_OPC_POP2() {

    long long temp;
    pop(&temp, TYPE_LONG);
}

static void exe_OPC_DUP() {
    int value;

    pop(&value, TYPE_INT);
    push(&value, TYPE_INT);
    push(&value, TYPE_INT);

    //memcpy(current_frame->ostack+1,current_frame->ostack, sizeof(int));
    //current_frame->ostack++;

}

static void exe_OPC_DUP_X1() {
    int value1, value2;
    pop(&value1, TYPE_INT);
    pop(&value2, TYPE_INT);

    push(&value1, TYPE_INT);
    push(&value2, TYPE_INT);
    push(&value1, TYPE_INT);

}

static void exe_OPC_DUP_X2() {

}
static void exe_OPC_DUP2() {
}
static void exe_OPC_DUP2_X1() {
}
static void exe_OPC_DUP2_X2() {
}
static void exe_OPC_SWAP() {
}

static void exe_OPC_IADD() {
    int value1, value2, result;

    pop(&value1, TYPE_INT);
    pop(&value2, TYPE_INT);
    result = value1 + value2;
    push(&result, TYPE_INT);

}

static void exe_OPC_LADD() {
    long long value1, value2, result;
    pop(&value2, TYPE_LONG);
    pop(&value1, TYPE_LONG);
    result = value1 + value2;
    push(&result, TYPE_LONG);

}

static void exe_OPC_FADD() {
    //TODO
    float value1, value2, result;
    pop(&value2, TYPE_FLOAT);
    pop(&value1, TYPE_FLOAT);
    result = value1 + value2;
    push(&result, TYPE_FLOAT);

}

static void exe_OPC_DADD() {
    double value1, value2, result;
    pop(&value2, TYPE_DOUBLE);
    pop(&value1, TYPE_DOUBLE);
    result = value1 + value2;
    push(&result, TYPE_DOUBLE);

}

static void exe_OPC_ISUB() {
    int value1, value2, result;
    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);
    result = value1 - value2;
    push(&result, TYPE_INT);

}

static void exe_OPC_LSUB() {
    long long value1, value2, result;
    pop(&value2, TYPE_LONG);
    pop(&value1, TYPE_LONG);
    result = value1 - value2;
    push(&result, TYPE_LONG);

}

static void exe_OPC_FSUB() {
    float value1, value2, result;
    pop(&value2, TYPE_FLOAT);
    pop(&value1, TYPE_FLOAT);
    result = value1 - value2;
    push(&result, TYPE_FLOAT);

}

static void exe_OPC_DSUB() {
    double value1, value2, result;
    pop(&value2, TYPE_DOUBLE);
    pop(&value1, TYPE_DOUBLE);
    result = value1 - value2;
    push(&result, TYPE_DOUBLE);

}

static void exe_OPC_IMUL() {
    int value1, value2, result;
    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);
    result = value1 * value2;
    push(&result, TYPE_INT);

}

static void exe_OPC_LMUL() {
    long long value1, value2, result;
    pop(&value2, TYPE_LONG);
    pop(&value1, TYPE_LONG);
    result = value1 * value2;
    push(&result, TYPE_LONG);

}

static void exe_OPC_FMUL() {
    float value1, value2, result;
    pop(&value2, TYPE_FLOAT);
    pop(&value1, TYPE_FLOAT);
    result = value1 * value2;
    push(&result, TYPE_FLOAT);

}

static void exe_OPC_DMUL() {
    double value1, value2, result;

    pop(&value2, TYPE_DOUBLE);
    pop(&value1, TYPE_DOUBLE);

    result = value1 * value2;

    push(&result, TYPE_DOUBLE);

}

static void exe_OPC_IDIV() {
    int value1, value2, result;
    pop(&value2, TYPE_INT);

    if(0 == value2)
      throwException("ArithmeticException");

    pop(&value1, TYPE_INT);
    result = value1/value2;
    push(&result, TYPE_INT);

}

static void exe_OPC_LDIV() {
    long long value1, value2, result;
    pop(&value2, TYPE_LONG);

    if (0 == value2)
      throwException("ArithmeticException");

    pop(&value1,TYPE_LONG);
    result = value1 / value2;
    push(&result, TYPE_LONG);

}

static void exe_OPC_FDIV() {
    float value1, value2, result;
    pop(&value2, TYPE_FLOAT);
    if (0 == value2)
      throwException("ArithmeticException");

    pop(&value1, TYPE_FLOAT);
    result = value1/value2;
    push(&result, TYPE_FLOAT);

}

static void exe_OPC_DDIV() {
    double value1, value2, result;

    pop(&value2, TYPE_DOUBLE);
    if (0 == value2)
      throwException("ArithmeticException");
    pop(&value1, TYPE_DOUBLE);
    result = value1/value2;
    push(&result, TYPE_DOUBLE);

}

static void exe_OPC_IREM() {
    /*NOTE: The JVM Specification define is
     *      value1-(value1/value2)*value2
     *@qcliu 2015/03/15
     */
    int value1, value2, result;
    pop(&value2, TYPE_INT);

    if (0 == value2)
      throwException("ArithmeticException");

    pop(&value1, TYPE_INT);
    result = value1 - (value1/value2)*value2;
    push(&result, TYPE_INT);

}

static void exe_OPC_LREM() {
    long long value1, value2, result;
    pop(&value2, TYPE_LONG);

    if (0 == value2)
      throwException("ArithmeticException");

    pop(&value1, TYPE_LONG);
    result = value1 % value2;
    push(&result, TYPE_LONG);

}

static void exe_OPC_FREM() {
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

static void exe_OPC_DREM() {
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

static void exe_OPC_INEG() {
    int value, result;
    pop(&value, TYPE_INT);
    result = 0 - value;
    push(&result, TYPE_INT);

}

static void exe_OPC_LNEG() {

    long long value, result;
    pop(&value, TYPE_LONG);
    result = 0 - value;
    push(&result, TYPE_LONG);
}

static void exe_OPC_FNEG() {

    float value, result;

    pop(&value, TYPE_FLOAT);
    result =-value;
    push(&result, TYPE_FLOAT);
}

static void exe_OPC_DNEG() {
    double value, result;

    pop(&value, TYPE_DOUBLE);
    result = -value;
    push(&result, TYPE_DOUBLE);

}

static void exe_OPC_ISHL() {
    int value1, value2, result;

    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);
    value2 = value2 & 0x0000001f;
    result = value1 << value2;
    push(&result, TYPE_INT);

}

static void exe_OPC_LSHL() {
    long long value1, result;
    int value2;

    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_LONG);
    value2 = value2 & 0x0000003f;
    result = value1 << value2;

    push(&result, TYPE_LONG);

}

static void exe_OPC_ISHR() {
    int value1, value2, result;

    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);
    value2 = value2 & 0x0000001f;
    result = value1 >> value2;
    push(&result, TYPE_INT);

}

static void exe_OPC_LSHR() {
    long long value1, result;
    int value2;

    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_LONG);
    value2 = value2 & 0x0000003f;
    result = value1 >> value2;

    push(&result, TYPE_LONG);

}

static void exe_OPC_IUSHR() {
    int value2, result;
    unsigned int value1;

    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_UINT);

    value2 = value2 & 0x0000001f;
    result = value1 >> value2;
    push(&result, TYPE_INT);

}

static void exe_OPC_LUSHR() {
    long long value1, result;
    int value2;

    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_ULONG);
    value2 = value2 & 0x0000003f;
    result = value1 >> value2;
    push(&result, TYPE_LONG);

}

static void exe_OPC_IAND() {
    int value1, value2;

    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);
    value1 = value1 & value2;
    push(&value1, TYPE_INT);

}

static void exe_OPC_LAND() {
    long long value1, value2;
    pop(&value2, TYPE_LONG);
    pop(&value1, TYPE_LONG);
    value1 = value1 & value2;
    push(&value1, TYPE_LONG);

}

static void exe_OPC_IOR() {
    int value1, value2;

    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);
    value1 = value1|value2;
    push(&value1, TYPE_INT);

}

static void exe_OPC_LOR() {
    long long value1, value2;

    pop(&value2, TYPE_LONG);
    pop(&value1, TYPE_LONG);
    value1 = value1 | value2;
    push(&value1, TYPE_LONG);

}
static void exe_OPC_IXOR() {
    int value1, value2;

    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);
    value1 = value1 ^ value2;
    push(&value1, TYPE_INT);

}

static void exe_OPC_LXOR() {
    long long value1, value2;

    pop(&value2, TYPE_LONG);
    pop(&value1, TYPE_LONG);
    value1 = value1 ^ value2;
    push(&value1, TYPE_LONG);

}

static void exe_OPC_IINC() {
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

static void exe_OPC_I2L() {
    int value;
    long long result;

    pop(&value, TYPE_INT);
    result = (long long)value;
    push(&result, TYPE_LONG);

}

static void exe_OPC_I2F() {
    int value;
    float result;

    pop(&value, TYPE_INT);
    result = (float)value;
    push(&result, TYPE_FLOAT);

}

static void exe_OPC_I2D() {
    int value;
    double result;

    pop(&value, TYPE_INT);
    result = (double)value;
    push(&result, TYPE_DOUBLE);

}

static void exe_OPC_L2I() {
    long long value;
    int result;

    pop(&value, TYPE_LONG);
    result = (int)value;
    push(&result, TYPE_INT);

}

static void exe_OPC_L2F() {
    long long value;
    float result;

    pop(&value, TYPE_LONG);
    result = (float)value;
    push(&result, TYPE_FLOAT);

}

static void exe_OPC_L2D() {
    long long value;
    double result;

    pop(&value, TYPE_LONG);
    result = (double)value;
    push(&result, TYPE_DOUBLE);

}

static void exe_OPC_F2I() {
    float value;
    int result;

    pop(&value, TYPE_FLOAT);
    result = (int)value;
    push(&result, TYPE_INT);

}

static void exe_OPC_F2L() {
    float value;
    long long result;

    pop(&value, TYPE_FLOAT);
    result = (long long)value;
    push(&result, TYPE_LONG);

}

static void exe_OPC_F2D() {
    float value;
    double result;

    pop(&value, TYPE_FLOAT);
    result = (double)value;
    push(&result, TYPE_DOUBLE);

}

static void exe_OPC_D2I() {
    double value;
    int result;

    pop(&value ,TYPE_DOUBLE);
    result = (int)value;
    push(&result, TYPE_INT);

}

static void exe_OPC_D2L() {
    double value;
    long long result;

    pop(&value ,TYPE_DOUBLE);
    result = (long long)value;
    push(&result, TYPE_LONG);

}

static void exe_OPC_D2F() {
    double value;
    float result;

    pop(&value, TYPE_DOUBLE);
    result = (float)value;
    push(&result, TYPE_FLOAT);

}

static void exe_OPC_I2B() {

}
static void exe_OPC_I2C() {
}
static void exe_OPC_I2S() {
}

static void exe_OPC_LCMP() {
    long long value1, value2;
    int result;

    pop(&value2, TYPE_LONG);
    pop(&value1, TYPE_LONG);
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
    push(&result, TYPE_INT);

}

static void exe_OPC_FCMPL() {
    float value1, value2;
    int result;

    pop(&value2, TYPE_FLOAT);
    pop(&value1, TYPE_FLOAT);
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
    push(&result, TYPE_INT);

}
static void exe_OPC_FCMPG() {
}
static void exe_OPC_DCMPL() {
}
static void exe_OPC_DCMPG() {
}

static void exe_OPC_IFEQ() {
    int value;
    pop(&value, TYPE_INT);
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

static void exe_OPC_IFNE() {
    int value;
    pop(&value, TYPE_INT);
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

static void exe_OPC_IFLT() {
    int value;
    pop(&value, TYPE_INT);
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

static void exe_OPC_IFGE() {
    int value;
    pop(&value, TYPE_INT);
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

static void exe_OPC_IFGT() {
    int value;
    pop(&value, TYPE_INT);
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

static void exe_OPC_IFLE() {
    int value;
    pop(&value, TYPE_INT);
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

static void exe_OPC_IF_ICMPEQ() {
    int value1, value2;

    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);

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

static void exe_OPC_IF_ICMPNE() {
    int value1, value2;
    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);

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

static void exe_OPC_IF_ICMPLT() {
    int value1, value2;
    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);
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
static void exe_OPC_IF_ICMPGE() {
    int value1, value2;
    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);
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

static void exe_OPC_IF_ICMPGT() {
    int value1, value2;
    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);
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

static void exe_OPC_IF_ICMPLE() {
    int value1, value2;
    pop(&value2, TYPE_INT);
    pop(&value1, TYPE_INT);
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

static void exe_OPC_IF_ACMPEQ() {
    Object *value1, *value2;
    pop(&value2, TYPE_REFERENCE);
    pop(&value1, TYPE_REFERENCE);

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

static void exe_OPC_IF_ACMPNE() {
    Object *value1, *value2;
    pop(&value2, TYPE_REFERENCE);
    pop(&value1, TYPE_REFERENCE);

    if (value1 != value2)
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

static void exe_OPC_GOTO() {
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

static void exe_OPC_JSR() {
    short offset;
    int address;

    PCMOVE(1);
    READ_IDX(offset, getCurrentPC());
    PCMOVE(1);

    address = offset;
    push(&address, TYPE_INT);

}

static void exe_OPC_RET() {

}

static void exe_OPC_TABLESWITCH() {
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

    pop(&index, TYPE_INT);
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

static void exe_OPC_LOOKUPSWITCH() {
    //TODO
}

static void exe_OPC_IRETURN() {
    //unsigned int* old_ostack = current_frame->prev->ostack;
    int value;
    pop(&value, TYPE_INT);
    //*((int*)&current_frame->ret) = value;
    returnValue(&value, TYPE_INT);
    //current_frame->prev->ostack++;
    //*(int*)current_frame->prev->ostack = value;

}
static void exe_OPC_LRETURN() {
}
static void exe_OPC_FRETURN() {
}

static void exe_OPC_DRETURN() {
    double value;
    pop(&value, TYPE_DOUBLE);

    returnValue(&value, TYPE_DOUBLE);
    //current_frame->prev->ostack++;
    //*(double*)current_frame->prev->ostack = value;
    //current_frame->prev->ostack++;

}

static void exe_OPC_ARETURN() {
    Object* array_ref;
    pop(&array_ref, TYPE_REFERENCE);
    //current_frame->prev->ostack++;
    //*(Object**)current_frame->prev->ostack = array_ref;
    returnValue(&array_ref, TYPE_REFERENCE);

}
static void exe_OPC_RETURN() {
}

static void exe_OPC_GETSTATIC() {
    u2 fieldref_idx;
    FieldBlock* fb;
    PCMOVE(1);
    READ_IDX(fieldref_idx, getCurrentPC());
    PCMOVE(1);
    fb = resolveField(getCurrentClass(), fieldref_idx);

    if (fb == NULL)
      throwException("resolveField return NULL");

    if (0 == strcmp(fb->type, "D"))
    {
        double value;
        value = *(double*)&(fb->static_value);
        push(&value, TYPE_DOUBLE);
    }
    else if (0 == strcmp(fb->type, "J"))
    {
        long long value;
        value = *(long long*)&(fb->static_value);
        push(&value, TYPE_LONG);
    }
    else if (0 == strcmp(fb->type, "F"))
    {
        float value;
        value = *(float*)&(fb->static_value);
        push(&value, TYPE_FLOAT);
    }
    else
    {
        int value;
        value = fb->static_value;
        push(&value, TYPE_INT);
    }

}

static void exe_OPC_PUTSTATIC() {
    u2 fieldref_idx;
    FieldBlock* fb;
    PCMOVE(1);
    READ_IDX(fieldref_idx, getCurrentPC());
    PCMOVE(1);
    fb = resolveField(getCurrentClass(), fieldref_idx);

    if (0 == strcmp(fb->type, "D"))
    {
        double value;
        pop(&value, TYPE_DOUBLE);
        *(double*)&(fb->static_value) = value;
    }
    else if (0 == strcmp(fb->type, "F"))
    {
        float value;
        pop(&value, TYPE_FLOAT);
        *(float*)&(fb->static_value) = value;
    }
    else if (0 == strcmp(fb->type, "J"))
    {
        long long value;
        pop(&value, TYPE_LONG);
        *(long long*)&(fb->static_value) = value;
    }
    else
    {
        int value;
        pop(&value, TYPE_INT);
        *(int*)&(fb->static_value) = value;
    }

}

static void exe_OPC_GETFIELD() {
    Object* objref;
    int value, offset;
    Object* value2;
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

    pop(&objref, TYPE_REFERENCE);
    if (0 == strcmp(fb->type, "J"))
    {
        long long value;
        value = OBJECT_DATA(objref, offset-1, long long);
        push(&value, TYPE_LONG);
    }
    else if (0 == strcmp(fb->type, "D"))
    {
        double value;
        value = OBJECT_DATA(objref, offset-1, double);
        push(&value, TYPE_DOUBLE);
    }
    else if (0 == strcmp(fb->type, "F"))
    {
        float value;
        value = OBJECT_DATA(objref, offset-1, float);
        push(&value, TYPE_FLOAT);
    }
    else
    {
        int value;
        value = OBJECT_DATA(objref, offset-1, int);
        push(&value, TYPE_INT);
    }

}

static void exe_OPC_PUTFIELD() {
    Object* objref;
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
        pop(&value, TYPE_LONG);
        pop(&objref, TYPE_REFERENCE);
        //*(long long*)&objref->data[offset-1] = value;
        OBJECT_DATA(objref, offset-1, long long) = value;
    }
    else if (0 == strcmp(fb->type, "D"))
    {
        double value;
        pop(&value, TYPE_DOUBLE);
        pop(&objref, TYPE_REFERENCE);
        //*(double*)&objref->data[offset-1] = value;
        OBJECT_DATA(objref, offset-1, double) = value;
    }
    else if (0 == strcmp(fb->type, "F"))
    {
        float value;
        pop(&value, TYPE_FLOAT);
        pop(&objref, TYPE_REFERENCE);
        //*(float*)&objref->data[offset-1] = value;
        OBJECT_DATA(objref, offset-1, float) = value;
    }
    else
    {
        int value;
        pop(&value, TYPE_INT);
        pop(&objref, TYPE_REFERENCE);
        //*(int*)&objref->data[offset-1] = value;
        OBJECT_DATA(objref, offset-1, int) = value;

    }


    /*note: The offset need sub 1, and the Exception is
     *      throw in resolveField().
     */

}

static void exe_OPC_INVOKEVIRTUAL() {
    u2 methodref_idx, name_type_idx, type_idx;
    MethodBlock* method;
    Object* objref;
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
                objref = *(Object**)(getCurrentFrame()->ostack - args_count);
                class = objref->class;
                ClassBlock* cb = CLASS_CB(class);

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

static void exe_OPC_INVOKESPECIAL() {
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

static void exe_OPC_INVOKESTATIC() {
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

static void exe_OPC_INVOKEINTERFACE() {
    u2 methodref_idx, name_type_idx, type_idx;
    MethodBlock* method;
    Object* objref;
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
                objref = *(Object**)(getCurrentFrame()->ostack - args_count);
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

static void exe_OPC_NEW() {
    u2 class_idx;
    C class;
    Object* obj;

    PCMOVE(1);
    READ_IDX(class_idx, getCurrentPC());
    PCMOVE(1);
    class = resolveClass(getCurrentClass(), class_idx);
    obj = allocObject(class);
    push(&obj, TYPE_REFERENCE);

}

static void exe_OPC_NEWARRAY() {
    int count, atype;
    Object* obj;
    PCMOVE(1);
    READ_BYTE(atype, getCurrentPC());
    pop(&count, TYPE_INT);

    obj = (Object*)allocTypeArray(atype, count, NULL);
    if (obj == NULL)
      throwException("newarray obj == NULL!");

    push(&obj, TYPE_REFERENCE);

}

static void exe_OPC_ANEWARRAY() {
    int count;
    u2 index;
    Object* obj;
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


    pop(&count, TYPE_INT);
    obj = (Object*)allocTypeArray(T_REFERENCE, count, ac_name);
    if (obj == NULL)
      throwException("anewarray obj == NULL!");

    push(&obj, TYPE_REFERENCE);

}

static void exe_OPC_ARRAYLENGTH() {
    Object* arrayref;
    int len;

    pop(&arrayref, TYPE_REFERENCE);

    C class = arrayref->class;
    ClassBlock* cb = CLASS_CB(class);

    if (0 == arrayref)
      throwException("NullPointerException");

    if (!arrayref->isArray)
      throwException("this obj is not a array.");

    len = arrayref->length;
    push(&len, TYPE_INT);

}

static void exe_OPC_ATHROW() {
    ClassBlock* cb = CLASS_CB(getCurrentClass());
    printf("current_class:%s, current_method:%s,%s",
                cb->this_classname, getCurrentMethodName(), getCurrentMethodDesc());

    //for test
    //printList(head);
    throwException("athrow test");
    Object* obj;

    pop(&obj, TYPE_REFERENCE);
    //TODO
    /*do some work*/
    push(&obj, TYPE_REFERENCE);

}

static void exe_OPC_CHECKCAST() {
    //TODO
    PCMOVE(1);
    PCMOVE(1);
}

static void exe_OPC_INSTANCEOF() {
    //TODO
    Object* obj;
    u2 index;
    int result;

    PCMOVE(1);
    READ_IDX(index, getCurrentPC());
    PCMOVE(1);

    pop(&obj, TYPE_REFERENCE);

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


    push(&result, TYPE_INT);

}

static void exe_OPC_MONITORENTER() {
    //TODO
    /*only for test*/
    Object* obj;
    pop(&obj, TYPE_REFERENCE);

}

static void exe_OPC_MONITOREXIT() {

    //TODO
    Object* obj;
    pop(&obj, TYPE_REFERENCE);
}
static void exe_OPC_WIDE() {
}
static void exe_OPC_MULTIANEWARRAY() {
}

static void exe_OPC_IFNULL() {
    /*the same as OPC_IF_ICMPGE
    */
    Object* obj;
    short offset;

    pop(&obj, TYPE_REFERENCE);

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

static void exe_OPC_IFNONNULL() {
    /*the same as OPC_IF_ICMPGE
    */
    Object* obj;
    short offset;

    pop(&obj, TYPE_REFERENCE);

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
static void exe_OPC_GOTO_W() {
}
static void exe_OPC_JSR_W() {
}

/*
 * invoke by: executeMethod(), executeStaticMethod() in execute.c
 */
void executeJava() {


    Frame* current_frame = getCurrentFrame();
    // When the stack frame is move to the top, exit.
    if (getCurrentFrame()->mb == NULL) {
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

    //unsigned int* min_stack_pointer = current_frame->ostack;
    //unsigned int* max_stack_pointer = current_frame->ostack + max_stack;
    //ClassBlock* cc = CLASS_CB(getCurrentClass());

    //Native doesn't have a return
    if (0 == code_length) {
        printf("code_length == 0    ");
        ClassBlock* cb = CLASS_CB(getCurrentClass());
        printf("method name:%s,type:%s, class:%s\n", getCurrentMethodName(),getCurrentMethodDesc(), cb->this_classname);

        exit(0);
        // popFrame();
        /*
         * After the popFrame(), must return executeJava().
         */
        return;
    }

    /*
     * note: pc_offset must move along with the current_pc. So use
     *       the PCMOVE() as much as possible.
     */
    while (getCurrentPCOffset() < code_length) {
        u4 c = *getCurrentPC();

        if (dis_testinfo) {
            printf("\nopcode: ---------%s\n", op_code[c]);//for test
            printf("pc_offset:------------%d\n", getCurrentPCOffset());
            printf("getCurrentPC():---------%d\n", (unsigned int)getCurrentPC());
            printStack();
        }
        switch (c) {
            case OPC_NOP://0
                break;
            case OPC_ACONST_NULL://1
                exe_OPC_ACONST_NULL();
                break;
            case OPC_ICONST_M1://2
                exe_OPC_ICONST_M1();
                break;
            case OPC_ICONST_0://3
                exe_OPC_ICONST_0();
                break;
            case OPC_ICONST_1://4
                exe_OPC_ICONST_1();
                break;
            case OPC_ICONST_2://5
                exe_OPC_ICONST_2();
                break;
            case OPC_ICONST_3://6
                exe_OPC_ICONST_3();
                break;
            case OPC_ICONST_4://7
                exe_OPC_ICONST_4();
                break;
            case OPC_ICONST_5://8
                exe_OPC_ICONST_5();
                break;
            case OPC_LCONST_0://9
                exe_OPC_LCONST_0();
                break;
            case OPC_LCONST_1: //10
                exe_OPC_LCONST_1();
                break;
            case OPC_FCONST_0: //11
                exe_OPC_FCONST_0();
                break;
            case OPC_FCONST_1: //12
                exe_OPC_FCONST_1();
                break;
            case OPC_FCONST_2: //13
                exe_OPC_FCONST_2();
                break;
            case OPC_DCONST_0://14
                exe_OPC_DCONST_0();
                break;
            case OPC_DCONST_1://15
                exe_OPC_DCONST_1();
                break;
            case OPC_BIPUSH://16
                exe_OPC_BIPUSH();
                break;
            case OPC_SIPUSH://17
                exe_OPC_SIPUSH();
                break;
            case OPC_LDC://18
                exe_OPC_LDC();
                break;
            case OPC_LDC_W://19
                exe_OPC_LDC_W();
                break;
            case OPC_LDC2_W://20
                exe_OPC_LDC2_W();
                break;

            case OPC_ILOAD://21
                exe_OPC_ILOAD();
                break;
            case OPC_LLOAD://22
                exe_OPC_LLOAD();
                break;
            case OPC_FLOAD://23
                exe_OPC_FLOAD();
                break;
            case OPC_DLOAD://24
                exe_OPC_DLOAD();
                break;
            case OPC_ALOAD://25
                exe_OPC_ALOAD();
                break;
            case OPC_ILOAD_0://26 NOTE: not test
                exe_OPC_ILOAD_0();
                break;
            case OPC_ILOAD_1://27
                exe_OPC_ILOAD_1();
                break;
            case OPC_ILOAD_2://28
                exe_OPC_ILOAD_2();
                break;
            case OPC_ILOAD_3://29
                exe_OPC_ILOAD_3();
                break;
            case OPC_LLOAD_0://30
                exe_OPC_LLOAD_0();
                break;
            case OPC_LLOAD_1://31
                exe_OPC_LLOAD_1();
                break;
            case OPC_LLOAD_2://32
                exe_OPC_LLOAD_2();
                break;
            case OPC_LLOAD_3://33
                exe_OPC_LLOAD_3();
                break;
            case OPC_FLOAD_0://34
                exe_OPC_FLOAD_0();
                break;
            case OPC_FLOAD_1://35
                exe_OPC_FLOAD_1();
                break;
            case OPC_FLOAD_2://36
                exe_OPC_FLOAD_2();
                break;
            case OPC_FLOAD_3://37
                exe_OPC_FLOAD_3();
                break;
            case OPC_DLOAD_0://38
                exe_OPC_DLOAD_0();
                break;
            case OPC_DLOAD_1://39
                exe_OPC_DLOAD_1();
                break;
            case OPC_DLOAD_2://40
                exe_OPC_DLOAD_2();
                break;
            case OPC_DLOAD_3://41
                exe_OPC_DLOAD_3();
                break;
            case OPC_ALOAD_0://42
                exe_OPC_ALOAD_0();
                break;
            case OPC_ALOAD_1://43
                exe_OPC_ALOAD_1();
                break;
            case OPC_ALOAD_2://44
                exe_OPC_ALOAD_2();
                break;
            case OPC_ALOAD_3://45
                exe_OPC_ALOAD_3();
                break;
            case OPC_IALOAD://46
                exe_OPC_IALOAD();
                break;
            case OPC_LALOAD://47
                exe_OPC_LALOAD();
                break;
            case OPC_FALOAD://48
                exe_OPC_FALOAD();
                break;
            case OPC_DALOAD://49
                exe_OPC_DALOAD();
                break;
            case OPC_AALOAD://50
                exe_OPC_AALOAD();
                break;
            case OPC_BALOAD://51
                exe_OPC_BALOAD();
                break;
            case OPC_CALOAD://52
                exe_OPC_CALOAD();
                break;
            case OPC_SALOAD://53
                exe_OPC_SALOAD();
                break;
            case OPC_ISTORE://54
                exe_OPC_ISTORE();
                break;
            case OPC_LSTORE://55
                exe_OPC_LSTORE();
                break;
            case OPC_FSTORE://56
                exe_OPC_FSTORE();
                break;
            case OPC_DSTORE://57
                exe_OPC_DSTORE();
                break;
            case OPC_ASTORE://58
                exe_OPC_ASTORE();
                break;
            case OPC_ISTORE_0://59
                exe_OPC_ISTORE_0();
                break;
            case OPC_ISTORE_1://60
                exe_OPC_ISTORE_1();
                break;
            case OPC_ISTORE_2://61
                exe_OPC_ISTORE_2();
                break;
            case OPC_ISTORE_3://62
                exe_OPC_ISTORE_3();
                break;
            case OPC_LSTORE_0://63
                exe_OPC_LSTORE_0();
                break;
            case OPC_LSTORE_1://64
                exe_OPC_LSTORE_1();
                break;
            case OPC_LSTORE_2://65
                exe_OPC_LSTORE_2();
                break;
            case OPC_LSTORE_3://66
                exe_OPC_LSTORE_3();
                break;
            case OPC_FSTORE_0://67
                exe_OPC_FSTORE_0();
                break;
            case OPC_FSTORE_1://68
                exe_OPC_FSTORE_1();
                break;
            case OPC_FSTORE_2://69
                exe_OPC_FSTORE_2();
                break;
            case OPC_FSTORE_3://70
                exe_OPC_FSTORE_3();
                break;
            case OPC_DSTORE_0://71
                exe_OPC_DSTORE_0();
                break;
            case OPC_DSTORE_1://72
                exe_OPC_DSTORE_1();
                break;
            case OPC_DSTORE_2://73
                exe_OPC_DSTORE_2();
                break;
            case OPC_DSTORE_3://74
                exe_OPC_DSTORE_3();
                break;
            case OPC_ASTORE_0://75
                exe_OPC_ASTORE_0();
                break;
            case OPC_ASTORE_1://76
                exe_OPC_ASTORE_1();
                break;
            case OPC_ASTORE_2://77
                exe_OPC_ASTORE_2();
                break;
            case OPC_ASTORE_3://78
                exe_OPC_ASTORE_3();
                break;
            case OPC_IASTORE://79
                exe_OPC_IASTORE();
                break;
            case OPC_LASTORE://80
                exe_OPC_LASTORE();
                break;
            case OPC_FASTORE://81
                exe_OPC_FASTORE();
                break;
            case OPC_DASTORE://82
                exe_OPC_DASTORE();
                break;
            case OPC_AASTORE://83
                exe_OPC_AASTORE();
                break;
            case OPC_BASTORE://84
                exe_OPC_BASTORE();
                break;
            case OPC_CASTORE://85
                exe_OPC_CASTORE();
                break;
            case OPC_SASTORE://86
                exe_OPC_SASTORE();
                break;
            case OPC_POP://87
                exe_OPC_POP();
                break;
            case OPC_POP2://88
                exe_OPC_POP2();
                break;
            case OPC_DUP://89
                exe_OPC_DUP();
                break;
            case OPC_DUP_X1://90
                exe_OPC_DUP_X1();
                break;
            case OPC_IADD://96
                exe_OPC_IADD();
                break;
            case OPC_LADD://97
                exe_OPC_LADD();
                break;
            case OPC_FADD://98
                exe_OPC_FADD();
                break;
            case OPC_DADD://99
                exe_OPC_DADD();
                break;
            case OPC_ISUB://100
                exe_OPC_ISUB();
                break;
            case OPC_LSUB://101
                exe_OPC_LSUB();
                break;
            case OPC_FSUB://102
                exe_OPC_FSUB();
                break;
            case OPC_DSUB://103
                exe_OPC_DSUB();
                break;
            case OPC_IMUL://104
                exe_OPC_IMUL();
                break;
            case OPC_LMUL://105
                exe_OPC_LMUL();
                break;
            case OPC_FMUL://106
                exe_OPC_FMUL();
                break;
            case OPC_DMUL://107
                exe_OPC_DMUL();
                break;
            case OPC_IDIV://108
                exe_OPC_IDIV();
                break;
            case OPC_LDIV://109
                exe_OPC_LDIV();
                break;
            case OPC_FDIV://110
                exe_OPC_FDIV();
                break;
            case OPC_DDIV://111
                exe_OPC_DDIV();
                break;
            case OPC_IREM://112
                exe_OPC_IREM();
                break;
            case OPC_LREM://113
                exe_OPC_LREM();
                break;
            case OPC_FREM://114
                DEBUG("TODO");
                exit(0);
                exe_OPC_FREM();
                break;
            case OPC_DREM://115
                DEBUG("TODO");
                exit(0);
                exe_OPC_DREM();
                break;
            case OPC_INEG://116
                exe_OPC_INEG();
                break;
            case OPC_LNEG://117
                exe_OPC_LNEG();
                break;
            case OPC_FNEG://118
                exe_OPC_FNEG();
                break;
            case OPC_DNEG://119
                exe_OPC_DNEG();
                break;
            case OPC_ISHL://120
                exe_OPC_ISHL();
                break;
            case OPC_LSHL://121
                exe_OPC_LSHL();
                break;
            case OPC_ISHR://122
                exe_OPC_ISHR();
                break;
            case OPC_LSHR://123
                exe_OPC_LSHR();
                break;
            case OPC_IUSHR://124
                exe_OPC_IUSHR();
                break;
            case OPC_LUSHR://125
                exe_OPC_LUSHR();
                break;
            case OPC_IAND://126
                exe_OPC_IAND();
                break;
            case OPC_LAND://127
                exe_OPC_LAND();
                break;
            case OPC_IOR://128
                exe_OPC_IOR();
                break;
            case OPC_LOR://129
                exe_OPC_LOR();
                break;
            case OPC_IXOR://130
                exe_OPC_IXOR();
                break;
            case OPC_LXOR://131
                exe_OPC_LXOR();
                break;
            case OPC_IINC://132
                exe_OPC_IINC();
                break;
            case OPC_I2L://133
                exe_OPC_I2L();
                break;
            case OPC_I2F://134
                exe_OPC_I2F();
                break;
            case OPC_I2D://135
                exe_OPC_I2D();
                break;
            case OPC_L2I://136
                exe_OPC_L2I();
                break;
            case OPC_L2F://137
                exe_OPC_L2F();
                break;
            case OPC_L2D://138
                exe_OPC_L2D();
                break;
            case OPC_F2I://139
                exe_OPC_F2I();
                break;
            case OPC_F2L://140
                exe_OPC_F2L();
                break;
            case OPC_F2D://141
                exe_OPC_F2D();
                break;
            case OPC_D2I://142
                exe_OPC_D2I();
                break;
            case OPC_D2L://143
                exe_OPC_D2L();
                break;
            case OPC_D2F://144
                exe_OPC_D2F();
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
                exe_OPC_LCMP();
                break;
            case OPC_FCMPG://150
            case OPC_FCMPL://149
                exe_OPC_FCMPL();
                break;
            case OPC_IFEQ://153
                exe_OPC_IFEQ();
                break;
            case OPC_IFNE://154
                exe_OPC_IFNE();
                break;
            case OPC_IFLT://155
                exe_OPC_IFLT();
                break;
            case OPC_IFGE://156
                exe_OPC_IFGE();
                break;
            case OPC_IFGT://157
                exe_OPC_IFGT();
                break;
            case OPC_IFLE://158
                exe_OPC_IFLE();
                break;
            case OPC_IF_ICMPEQ://159
                exe_OPC_IF_ICMPEQ();
                break;
            case OPC_IF_ICMPNE://160
                exe_OPC_IF_ICMPNE();
                break;
            case OPC_IF_ICMPLT://161
                exe_OPC_IF_ICMPLT();
                break;
            case OPC_IF_ICMPGE://162
                exe_OPC_IF_ICMPGE();
                break;
            case OPC_IF_ICMPGT://163
                exe_OPC_IF_ICMPGT();
                break;
            case OPC_IF_ICMPLE://164
                exe_OPC_IF_ICMPLE();
                break;
            case OPC_IF_ACMPEQ://165
                exe_OPC_IF_ACMPEQ();
                break;
            case OPC_IF_ACMPNE://166
                exe_OPC_IF_ACMPNE();
                break;
            case OPC_GOTO://167
                exe_OPC_GOTO();
                break;
            case OPC_JSR://168
                exe_OPC_JSR();
                break;
            case OPC_TABLESWITCH://170
                exe_OPC_TABLESWITCH();
                break;
            case OPC_LOOKUPSWITCH:
                DEBUG("TODO");
                exit(0);
                break;
            case OPC_IRETURN://172
                exe_OPC_IRETURN();
                return;
                break;
            case OPC_FRETURN://174
                throwException("todo");
                break;
            case OPC_DRETURN://175
                exe_OPC_DRETURN();
                return;
                break;
            case OPC_ARETURN://176
                exe_OPC_ARETURN();
                return;
                break;
            case OPC_RETURN://177
                //printf("---------------current  stack top:%d\n", *(int*)current_frame->ostack);
                //popFrame();
                return;
                break;
            case OPC_GETSTATIC://178
                exe_OPC_GETSTATIC();
                break;
            case OPC_PUTSTATIC://179
                exe_OPC_PUTSTATIC();
                break;
            case OPC_GETFIELD://180
                exe_OPC_GETFIELD();
                break;
            case OPC_PUTFIELD://181
                exe_OPC_PUTFIELD();
                break;
            case OPC_INVOKEVIRTUAL://182
                exe_OPC_INVOKEVIRTUAL();
                break;
                /*
                 * First, should determine the class is current class or
                 * super class.
                 */
            case OPC_INVOKESPECIAL://183
                exe_OPC_INVOKESPECIAL();
                break;
            case OPC_INVOKESTATIC://184
                exe_OPC_INVOKESTATIC();
                break;
            case OPC_INVOKEINTERFACE://185
                exe_OPC_INVOKEINTERFACE();
                break;
            case OPC_NEW://187
                exe_OPC_NEW();
                break;
            case OPC_NEWARRAY://188
                exe_OPC_NEWARRAY();
                break;
            case OPC_ANEWARRAY://189
                exe_OPC_ANEWARRAY();
                break;
            case OPC_ARRAYLENGTH://190
                exe_OPC_ARRAYLENGTH();
                break;
            case OPC_ATHROW://191
                exe_OPC_ATHROW();
                break;
            case OPC_CHECKCAST://192
                exe_OPC_CHECKCAST();
                break;
            case OPC_INSTANCEOF://193
                exe_OPC_INSTANCEOF();
                break;
            case OPC_MONITORENTER://194
                exe_OPC_MONITORENTER();
                break;
            case OPC_MONITOREXIT://195
                exe_OPC_MONITOREXIT();
                break;
            case OPC_IFNULL://198
                exe_OPC_IFNULL();
                break;
            case OPC_IFNONNULL://199
                exe_OPC_IFNONNULL();
                break;
            default:
                printStack();
                printf("\nwrong opcode!!%s\n", op_code[c]);
                exit(0);
        }
        PCMOVE(1);
    }


}

#undef C
