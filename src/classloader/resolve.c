/*^_^*--------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  resolve.c                              */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2015/01/04                             */
/*  DESCRIPTION           :  to resovle class,method ,etc           */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 *2015/01/06
 *Add findMethod()
 *//*}}}*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../main/turkey.h"
#include "resolve.h"
#include "class.h"
#include "../control/control.h"
#include "../util/exception.h"
#include "../interp/stackmanager.h"
#include "../lib/error.h"
#include "../lib/assert.h"

#define C Class_t
#define JF JFrame_t

/*
 * Find field in current class.
 */
static FieldBlock_t* findFieldinCurrent(C class, char* name, char* type)
{
    ClassBlock_t* cb = CLASS_CB(class);/*{{{*/
    int i;

    for (i = 0; i<cb->fields_count; i++) {
        FieldBlock_t* fb = &cb->fields[i];
        if ((strcmp(fb->name, name) == 0) && (strcmp(fb->type, type)== 0))
            return fb;
    }
    return 0;/*}}}*/
}

/*
 * Find field in current, if not, find in super recursivly.
 */
FieldBlock_t* findField(C class, char* name, char* type)
{
    if (class == NULL)/*{{{*/
        DEBUG("NULLPointer error");

    ClassBlock_t* cb = CLASS_CB(class);
    FieldBlock_t* fb = NULL;
    fb = findFieldinCurrent(class, name, type);
    if (fb != NULL)
        return fb;
    if (!cb->super)
        return fb;
    return findField(cb->super, name, type);/*}}}*/
}

/*
 * Given the Fieldinfo_idx, and find the field from the given
 * class resursivly. Finally, return the offset of the field
 * for a certain object. The objectref has been in the stack.
 *
 * note: the offset start with 1. Take 0 as failure case. So
 *       before use the offset, remember offset-=1.
 * @qcliu 2015/01/30
 */
FieldBlock_t* resolveField(C class, u2 index)
{
    /*{{{*/
    FieldBlock_t* fb = NULL;
    ConstantPool_t* current_cp;

    JF current_frame = getCurrentFrame();
    current_cp = GET_CONSTANTPOOL(current_frame);
    switch (CP_TYPE(current_cp, index)) {
    case CONSTANT_Fieldref: {
        u4 cp_info;
        u2 name_type_idx;
        u2 symclass_idx;
        u2 name_idx;
        u2 type_idx;
        char* name;
        char* type;
        C sym_class;

        symclass_idx = 0;
        cp_info = CP_INFO(current_cp, index);
        symclass_idx = cp_info;
        sym_class = (C)resolveClass(GET_CLASS(current_frame), symclass_idx);
        name_type_idx = cp_info >> 16;
        cp_info = CP_INFO(current_cp, name_type_idx);
        name_idx = cp_info;
        type_idx = cp_info >> 16;
        name = CP_UTF8(current_cp, name_idx);
        type = CP_UTF8(current_cp, type_idx);
        ClassBlock_t* cb = CLASS_CB(class);

        fb = findField(sym_class, name, type);
        /*
        *(FieldBlock_t**)&CP_INFO(current_cp, index) = fb;
        CP_TYPE(current_cp, index) = RESOLVED;
        */
        break;
    }
    case RESOLVED: {
        ERROR("impossible");
        fb = (FieldBlock_t*)CP_INFO(current_cp, index);
        break;
    }
    default: {
        printf("resolveFiled error\n!");
        exit(0);
    }
    }
    return fb;/*}}}*/
}

/*
 * Find method in current class.
 * If not found, return NULL.
 */
MethodBlock_t* findMethodinCurrent(C class, char* name, char* type)
{
    ClassBlock_t* cb = CLASS_CB(class);/*{{{*/
    int i;

    for (i = 0; i<cb->methods_count; i++) {
        MethodBlock_t* mb = &cb->methods[i];
        if ((strcmp(mb->name, name) == 0) && (strcmp(mb->type, type) == 0))
            return mb;
    }
    return NULL;/*}}}*/
}
/*
 * find the method in current class, if not ,find in super recursivly.
 * note: find the method in ClassBlock_t, not the vtable.
 */
MethodBlock_t* findMethod(C class, char* name, char* type)
{
    ClassBlock_t* cb = CLASS_CB(class); /*{{{*/
    MethodBlock_t* mb = findMethodinCurrent(class, name, type);

    if (mb != NULL)
        return mb;
    if (!cb->super)
        return mb;
    return findMethod(cb->super, name, type);/*}}}*/
}

/**
 * Find method in vtable.
 * @see exe_OPC_INVOKEVIRTUAL()
 */
MethodBlock_t* quickSearch(C class, char* name, char* type)
{
    /*{{{*/
    ClassBlock_t* cb;
    MethodBlock_t** methods_table;
    u2 methods_table_size;

    cb = CLASS_CB(class);
    methods_table = cb->methods_table;
    methods_table_size = cb->methods_table_size;
    Assert_ASSERT(methods_table);
    int i;
    for (i=0; i<methods_table_size; i++) {
        MethodBlock_t* mb = *(methods_table+i);
        if (!(0 == strcmp(name, mb->name) &&
                0 == strcmp(type, mb->type)))
            continue;

        return mb;
    }
    return NULL;
    /*}}}*/
}

/*
 * According to the Classinfo in constants_pool, find
 * the address of a certain class. It's different with
 * resolveMethod, because it dosen't refer to the runtime
 * constans_pool. Yet, may be the same with runtime cp by
 * chance.
 *
 * note: the args means that the index must belong to the
 *       class's cp. So if you want to get an address of
 *       a class but don't know the CONSTANT_Class belong
 *       to which class, you should use loadClass().
 *
 *  @qcliu 2015/01/25
 **/
C resolveClass(C class,  u2 index)
{
    /*{{{*/
    C resolve_class = NULL;
    ClassBlock_t* cb = CLASS_CB(class);
    ConstantPool_t* cp = &cb->constant_pool;

    switch (CP_TYPE(cp, index)) {
        char* classname;

    case CONSTANT_Class: {
        classname = CP_UTF8(cp, CP_INFO(cp, index));
        resolve_class = loadClass(classname);
        /*
        *(C*)&CP_INFO(cp, index) = resolve_class;
        CP_TYPE(cp, index) = RESOLVED;
        */
        break;
    }
    case RESOLVED: {
        resolve_class = (C)CP_INFO(cp, index);
        break;
    }
    default:
        printf("resolveClass error!!\n");
        printf("%d\n", CP_TYPE(cp, index));
        exit(0);
    }
    return resolve_class; /*}}}*/
}

/*
* The interfacemethod is different with normal method. Because
* an interface may have several implementation. So the InterfaceMethodref
* in the current_cp can not determine. This is why HotSpot use "ITable".
*
* In our turkey, we just find method, don't need rewrite the constants_pool.
*
* @qcliu 2015/01/26
*/
MethodBlock_t* resolveInterfaceMethod(C class, u2 index)
{
    /*{{{*/
    MethodBlock_t* resolve_method = NULL;
    JF current_frame = getCurrentFrame();
    ConstantPool_t* current_cp = GET_CONSTANTPOOL(current_frame);

    switch (CP_TYPE(current_cp, index)) {
    case CONSTANT_InterfaceMethodref: {
        u4 cp_info;
        u2 name_type_idx;
        u2 name_idx;
        u2 type_idx;
        char *name;
        char *type;

        cp_info = CP_INFO(current_cp, index);
        /*high                    low
         *--------------------------
         *|name&type |class        |
         *--------------------------
         */
        name_type_idx = cp_info >> 16;
        cp_info = CP_INFO(current_cp, name_type_idx);
        name_idx = cp_info;
        type_idx = cp_info>>16;
        name = CP_UTF8(current_cp, name_idx);
        type = CP_UTF8(current_cp, type_idx);
        resolve_method = (MethodBlock_t*)findMethod(class, name, type);
        if (resolve_method == NULL) {
            ClassBlock_t* cb = CLASS_CB(class);
            printf("%s %s in %s\n", name, type, cb->this_classname);
            ERROR("resolveInterfaceMethod");
            throwException("no such method");
        }
        /*NOTE: interfaceMethod no need to change the tag*/
        /*
         *(MethodBlock_t**)&CP_INFO(current_cp, index) = resolve_method;
         CP_TYPE(current_cp, index) = RESOLVED;
         */
        break;
    }
    case RESOLVED: {
        throwException("resolveInterfaceMethod resolved");
        break;
    }
    default: {
        printf("resolve method error!!!\n");
        printf("%d\n", CP_TYPE(current_cp, index));
        exit(0);
    }
    }
    return resolve_method;
    /*}}}*/
}

/*
 * Give a index(Methodinfo_rf), find method in current_cp,
 * if not find, find method from the class given by first arg,
 * else return the address of MethodBlock_t.
 *
 * note: the first arg class must be already inited.
 *
 * @qcliu 2015/01/26
 */
MethodBlock_t* resolveMethod(C class, u2 index, MethodBlock_t*(*f)(C, char*, char*))
{
    /*{{{*/
    MethodBlock_t* resolve_method = NULL;
    JF current_frame = getCurrentFrame();
    ConstantPool_t* current_cp = GET_CONSTANTPOOL(current_frame);

    switch (CP_TYPE(current_cp, index)) {
    case CONSTANT_Methodref: {
        u4 cp_info;
        u2 name_type_idx;
        u2 name_idx;
        u2 type_idx;
        char* name;
        char* type;

        cp_info = CP_INFO(current_cp, index);
        /*high                    low
         *--------------------------
         *|name&type |class        |
         *--------------------------
         */
        name_type_idx = cp_info >> 16;
        cp_info = CP_INFO(current_cp, name_type_idx);
        name_idx = cp_info;
        type_idx = cp_info>>16;
        name = CP_UTF8(current_cp, name_idx);
        type = CP_UTF8(current_cp, type_idx);
        resolve_method = (MethodBlock_t*)f(class, name, type);
        if (resolve_method == NULL) {
            ClassBlock_t* cb = CLASS_CB(class);
            printf("%s %s in %s\n", name, type, cb->this_classname);
            ERROR("resolveMethod");
            throwException("no such method");
        }
        //fprintf(stdout, "resovle method: %x\n", resolve_method);
        /*change the tag to record that the method is already resovled*/
        //*(MethodBlock_t**)&CP_INFO(current_cp, index) = resolve_method;
        //CP_TYPE(current_cp, index) = RESOLVED;
        // fprintf(stdout, "cp->info[index]: %x\n", CP_INFO(current_cp, index));
        // fprintf(stdout, "cp->type[index]: %x\n", CP_TYPE(current_cp, index));
        break;
    }
    case RESOLVED: {
        ERROR("impossible");
        resolve_method = (MethodBlock_t*)CP_INFO(current_cp, index);
        break;
    }
    default: {
        printf("resolve method error!!!\n");
        printf("%d\n", CP_TYPE(current_cp, index));
        exit(0);
    }
    }
    return resolve_method;
    /*}}}*/
}

u4 resolveConstant(C class, int cp_index)
{
    Assert_ASSERT(class);
    ClassBlock_t* cb = CLASS_CB(class);
    ConstantPool_t* cp = &cb->constant_pool;

    switch (CP_TYPE(cp, cp_index)) {
    case CONSTANT_String:
        TODO("String");
        break;
    default:
        break;
    }
    return CP_INFO(cp, cp_index);
}

#undef C
#undef JF
