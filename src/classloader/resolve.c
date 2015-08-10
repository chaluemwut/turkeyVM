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
static FieldBlock* findFieldinCurrent(C class, char* name, char* type)
{
    ClassBlock* cb = CLASS_CB(class);/*{{{*/
    int i;

    for (i = 0; i<cb->fields_count; i++)
    {
        FieldBlock* fb = &cb->fields[i];
        if ((strcmp(fb->name, name) == 0) && (strcmp(fb->type, type)== 0))
            return fb;
    }

    return 0;/*}}}*/
}

/*
 * Find field in current, if not, find in super recursivly.
 */
FieldBlock* findField(C class, char* name, char* type)
{
    if (class == NULL)/*{{{*/
        DEBUG("NULLPointer error");

    ClassBlock* cb = CLASS_CB(class);
    FieldBlock* fb = NULL;
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
FieldBlock* resolveField(C class, u2 index)
{
    /*{{{*/
    FieldBlock* fb = NULL;
    ConstantPool* current_cp;

    JF current_frame = getCurrentFrame();
    current_cp = getCurrentCP();

    switch (CP_TYPE(current_cp, index))
    {
    case CONSTANT_Fieldref:
    {
        u4 cp_info;
        u2 name_type_idx, symclass_idx, name_idx, type_idx;
        char *name, *type;
        C sym_class;

        symclass_idx = 0;
        cp_info = CP_INFO(current_cp, index);
        symclass_idx = cp_info;
        sym_class = (C)resolveClass(getCurrentClass(), symclass_idx);
        name_type_idx = cp_info >> 16;
        cp_info = CP_INFO(current_cp, name_type_idx);
        name_idx = cp_info;
        type_idx = cp_info >> 16;

        name = CP_UTF8(current_cp, name_idx);
        type = CP_UTF8(current_cp, type_idx);

        ClassBlock* cb = CLASS_CB(class);

        //if (dis_testinfo)
        //{
        //   printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~%s\n", cb->this_classname);
        //  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~fields_count:%d\n", cb->fields_count);
        // }

        fb = findField(sym_class, name, type);


        /*
        *(FieldBlock**)&CP_INFO(current_cp, index) = fb;
        CP_TYPE(current_cp, index) = RESOLVED;

        */
        break;
    }
    case RESOLVED:
    {
        fb = (FieldBlock*)CP_INFO(current_cp, index);
        break;
    }
    default:
    {
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
MethodBlock* findMethodinCurrent(C class, char* name, char* type)
{
    ClassBlock* cb = CLASS_CB(class);/*{{{*/
    int i;

    for (i = 0; i<cb->methods_count; i++)
    {
        MethodBlock* mb = &cb->methods[i];
        if ((strcmp(mb->name, name) == 0) && (strcmp(mb->type, type) == 0))
            return mb;
    }

    return NULL;/*}}}*/
}
/*
 * find the method in current class, if not ,find in super recursivly.
 * note: find the method in ClassBlock, not the vtable.
 */
MethodBlock* findMethod(C class, char* name, char* type)
{
    ClassBlock* cb = CLASS_CB(class); /*{{{*/
    MethodBlock* mb = findMethodinCurrent(class, name, type);

    if (mb != NULL)
        return mb;


    if (!cb->super)
        return mb;

    return findMethod(cb->super, name, type);/*}}}*/
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
    ClassBlock* cb = CLASS_CB(class);
    ConstantPool* cp = &cb->constant_pool;

    switch (CP_TYPE(cp, index))
    {
        char* classname;

    case CONSTANT_Class:
    {
        classname = CP_UTF8(cp, CP_INFO(cp, index));
        resolve_class = loadClass(classname);

        /*
        *(C*)&CP_INFO(cp, index) = resolve_class;
        CP_TYPE(cp, index) = RESOLVED;
        */

        break;
    }
    case RESOLVED:
    {
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
 *
 *
 *
 *  @qcliu 2015/05/10
 */
MethodBlock* resolveInterfaceMethod(C class, u2 index)
{
    MethodBlock* resolve_method = NULL;
    JF current_frame = getCurrentFrame();
    ConstantPool* current_cp = getCurrentCP();


    switch (CP_TYPE(current_cp, index))
    {
    case CONSTANT_InterfaceMethodref:
    {
        u4 cp_info;
        u2 name_type_idx, name_idx, type_idx;
        char *name, *type;

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

        resolve_method = (MethodBlock*)findMethod(class, name, type);

        if (resolve_method == NULL)
        {
            ClassBlock* cb = CLASS_CB(class);
            printf("%s %s in %s\n", name, type, cb->this_classname);
            ERROR("resolveInterfaceMethod");
            throwException("no such method");
        }

        /*NOTE: interfaceMethod no need to change the tag*/
        /*
        *(MethodBlock**)&CP_INFO(current_cp, index) = resolve_method;
        CP_TYPE(current_cp, index) = RESOLVED;
        */
        break;
    }
    case RESOLVED:
    {
        throwException("resolveInterfaceMethod resolved");
        break;
    }
    default:
    {
        printf("resolve method error!!!\n");
        printf("%d\n", CP_TYPE(current_cp, index));
        exit(0);
    }
    }

    return resolve_method;
}

/*
 * Give a index(Methodinfo_rf), find method in current_cp,
 * if not find, find method from the class given by first arg,
 * else return the address of MethodBlock.
 *
 * note: the first arg class must be already inited.
 *
 * @qcliu 2015/01/26
 */
MethodBlock* resolveMethod(C class, u2 index)
{
    MethodBlock* resolve_method = NULL;/*{{{*/
    JF current_frame = getCurrentFrame();
    ConstantPool* current_cp = getCurrentCP();


    switch (CP_TYPE(current_cp, index))
    {
    case CONSTANT_Methodref:
    {
        u4 cp_info;
        u2 name_type_idx, name_idx, type_idx;
        char *name, *type;

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

        resolve_method = (MethodBlock*)findMethod(class, name, type);

        if (resolve_method == NULL)
        {
            ClassBlock* cb = CLASS_CB(class);
            printf("%s %s in %s\n", name, type, cb->this_classname);
            ERROR("resolveMethod");
            throwException("no such method");
        }

        /*change the tag to record that the method is already resovled*/
        /*
        *(MethodBlock**)&CP_INFO(current_cp, index) = resolve_method;
        CP_TYPE(current_cp, index) = RESOLVED;
        */
        break;
    }
    case RESOLVED:
    {
        resolve_method = (MethodBlock*)CP_INFO(current_cp, index);
        break;
    }
    default:
    {
        printf("resolve method error!!!\n");
        printf("%d\n", CP_TYPE(current_cp, index));
        exit(0);
    }
    }


    return resolve_method;
    /*}}}*/
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
/*MethodBlock* resolveInterfaceMethod(C class, u2 index)
  {
  MethodBlock* resolve_method = NULL;
  ConstantPool* current_cp = getCurrentCP();

  switch (CP_TYPE(current_cp, index))
  {
  case CONSTANT_InterfaceMethodref:
  {
  u4 cp_info;
  u2 name_type_idx, name_idx, type_idx;
  char *name, *type;

  cp_info = CP_INFO(current_cp, index);
  name_type_idx = cp_info;
  cp_info = CP_INFO(current_cp, name_type_idx);
  name_idx = cp_info;
  type_idx = cp_info >> 16;
  name = CP_UTF8(current_cp, name_idx);
  type = CP_UTF8(current_cp, type_idx);

  resolve_method = (MethodBlock*)findMethod(class, name, type);
  break;
  }
  default:
  printf("resolve interfacemethod error!!!\n");
  printf("%d\n", CP_TYPE(current_cp, index));
  exit(0);
  }

  return resolve_method;
  }*/



/*
 * Resolve virtual method. Find method in the VTable of the class
 * that given by first arg.
 */
MethodBlock* resolveVirtualMethod(C class, u2 index)
{
    /*
     * TODO
     */
    printf("TODO\n");
    return NULL;
}

u4 resolveConstant(C class, int cp_index)
{
    Assert_ASSERT(class);
    ClassBlock* cb = CLASS_CB(class);
    ConstantPool* cp = &cb->constant_pool;

    switch (CP_TYPE(cp, cp_index))
    {
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
