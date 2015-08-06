/*------------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  testvm.c                               */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2014/01/30                             */
/*  DESCRIPTION           :  for the javaVM                         */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 *
 *//*}}}*/

#include "testvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exception.h"
#include "../classloader/resolve.h"
#include "../main/turkey.h"
#include "../interp/stackmanager.h"

/*packing the print operation*/
/*{{{*/
#define PRINT_DISTANCE(d)    \
    if(distance>d)                 \
        throwException("IndentError");          \
    for(;distance < d;distance++)  \
    {                               \
        printf(" ");                 \
    }

#define PRINT_SPACE             \
        t = indentLevel;    \
        for(;t > 0;t--)          \
          distance += printf(" ")
//"..."&...
#define PRINT_D(d,s)        distance += printf(d"%d",s)
#define PRINT_DS(d,s)       PRINT_SPACE;distance += printf(d"%d",s)
#define PRINT_X(d,s)        distance += printf(d"%x",s)
#define PRINT_XS(d,s)       PRINT_SPACE;distance += printf(d"%x",s)
#define PRINT_S(d,s)        distance += printf(d"%s",s)
#define PRINT_SS(d,s)       PRINT_SPACE;distance += printf(d"%s",s)
//"..."
#define PRINT_TAB           distance = printf("    ")
#define PRINT(s)            distance += printf(s)
#define PRINT_SP(s)         PRINT_SPACE;distance += printf(s)
//clear the distance
#define PRINTLN(s)          distance = 0;printf(s"\n")
#define PRINTLN_S(s)        PRINT_SPACE;distance = 0;printf(s"\n")
#define INDENT indentLevel+=2
#define UNINDENT indentLevel-=2
/*}}}*/

#define C Class_t
#define O Object_t
#define JF JFrame_t
#define NF NFrame_t

extern C java_lang_String;
static int distance, t, indentLevel;


/*
void printList(LinkedList* head)
{
    void* temp = getFirst(head);
    LNode* ptr = GETLINK(head)->next;

    int size = getLength(head);
    for (; ptr != NULL; ptr = ptr->next)
    {
        int i;
        C class = (C)ptr->data;
        ClassBlock* cb = CLASS_CB(class);
        printf("classname: %s\n",cb->this_classname);
    }

    printf("loadedTable size: %d\n", size);
}
*/



/*
 * Print the Vtable of the class that alreay be loaded.
 * @qcliu 2015/01/30
 */
/*
void printVtable(LinkedList* head)
{
    void* temp = getFirst(head);
    LNode* ptr = GETLINK(head)->next;

    for (; ptr != NULL; ptr = ptr->next)
    {
        int i;
        C class = (C)ptr->data;
        ClassBlock* cb = CLASS_CB(class);
        int vtable_size = cb->methods_table_size;
        MethodBlock** vtable  = cb->methods_table;
        char* ele_name = "null";
        printf("\nclassname: %s, vtable_size: %d, object_size: %d\n",cb->this_classname,vtable_size, cb->obj_size);
        if (cb->element != NULL)
        {
            ClassBlock* ele_cb = CLASS_CB(cb->element);
            ele_name = ele_cb->this_classname;
        }
        printf("element: %s, dim: %d\n", ele_name, cb->dim);

        if(vtable == NULL)
          continue;
        PRINT("idx");
        PRINT_DISTANCE(5);
        PRINT("name");
        PRINT_DISTANCE(35);
        PRINT("type");
        PRINTLN("");
        for (i = 0; i<vtable_size; i++)
        {
            int distance = 0;
            MethodBlock* mb = vtable[i];
            int midx = mb->methods_table_idx;
            PRINT_D("|",midx);
            PRINT_DISTANCE(5);
            PRINT_S("|",mb->name);
            PRINT_DISTANCE(35);
            PRINT_S("|",mb->type);
            PRINTLN("");
        }
    }

}
*/

/*
 * Print the current stack and locals.
 * @qcliu 2015/01/30
 *
 *    top
 *     ^
 *     |
 *     |
 *     |
 *     bottom
 */
void printNativeStack()
{
    NF nframe = getNativeFrame();
    printf("Native locals\n");
    MethodBlock* nmb = nframe->mb;
    int count = nmb->max_locals;
    int i;

    for (i = 0; i < count; i++)
        printf("%d\n", nframe->locals[i]);

}
void printStack(JF current_frame)
{
    //JF current_frame = getCurrentFrame();
    unsigned int* postack;/*{{{*/
    int max_stack;
    int max_locals;
    int locals_idx;

    /*print statck*/
    max_stack = current_frame->mb->max_stack;
    max_locals = current_frame->mb->max_locals;
    postack = current_frame->locals + max_stack + max_locals - 1;

    PRINTLN("");
    PRINT_DISTANCE(10);
    PRINTLN("stack");
    for (; max_stack > 0; max_stack--)
    {
        if (postack == current_frame->ostack)
        {
            PRINT_DISTANCE(5);
            PRINT("top->");
        }
        PRINT_DISTANCE(10);
        PRINTLN("----------------");
        PRINT_DISTANCE(10);
        PRINT_D("|", *(int*)postack);
        PRINT_DISTANCE(25);
        PRINTLN("|");
        postack--;
    }

    PRINT_DISTANCE(4);
    PRINT("bottom");
    PRINTLN("----------------");

    /*print locals*/
    PRINTLN("");
    PRINT_DISTANCE(10);
    PRINTLN("locals");
    postack = current_frame->locals;
    for (locals_idx = 0; max_locals > 0; max_locals--, locals_idx++)
    {
        PRINT_DISTANCE(10);
        PRINTLN("----------------");
        PRINT_DISTANCE(7);
        PRINT("[");
        PRINT_D("", locals_idx);
        PRINT("]");
        PRINT_D("|", *(int*)postack);
        PRINT_DISTANCE(25);
        PRINTLN("|");
        postack++;
    }
    PRINT_DISTANCE(10);
    PRINTLN("----------------");
    /*}}}*/
}

void printArray(O arrayref)
{
    C class;
    ClassBlock* cb;
    int i;
    int el_size;

    class = arrayref->class;
    cb = CLASS_CB(class);
    el_size = arrayref->el_size;
    void* data = arrayref->data;

    for(i=0; i<arrayref->length; i++)
    {
        switch (arrayref->atype)
        {
        case T_CHAR:
            printf("%d|%d\n",i, *((u2*)arrayref->data+el_size*i));
            break;
        case T_BOOLEAN:
            printf("%d|%d\n", i, *((int*)arrayref->data+el_size*i));
            break;
        case T_DOUBLE:
            printf("%d|%lf\n", i, *((double*)arrayref->data+el_size*i));
            break;
        case T_BYTE:
            printf("%d|%c\n", i, *((char*)arrayref->data+el_size*i));
            break;
        case T_FLOAT:
            printf("%d|%f\n", i, *((float*)arrayref->data+el_size*i));
            break;
        case T_INT:
            printf("%d|%d\n", i, *((int*)arrayref->data+el_size*i));
            break;
        case T_LONG:
            printf("%d|%llu\n", i, *((long long*)arrayref->data+el_size*i));
            break;
        case T_SHORT:
            printf("%d|%d\n", i, *((u2*)arrayref->data+el_size*i));
            break;
        case T_REFERENCE:
            printf("%d|%p\n", i, *(void**)((int*)arrayref->data+el_size*i));
            break;
        default:
            throwException("atype error");
        }
    }

}

void printObjectWrapper(O objref)
{
    if (objref == NULL)
        throwException("printObjectWrapper: arg is NULL");

    if (objref->isArray == 1)
        printArray(objref);
    else
        printObject(objref);
}
/*
 * Print the Object
 * @qcliu 2015/01/30
 */
void printObject(O objref)
{
    C class;/*{{{*/
    ClassBlock* cb;
    int obj_size;
    char* classname;
    int i;

    class = objref->class;
    cb = CLASS_CB(class);
    obj_size = cb->obj_size;
    classname = cb->this_classname;

    PRINTLN("");
    PRINT_DISTANCE(11);
    PRINT_S("",classname);
    PRINTLN("");

    PRINT_DISTANCE(11);
    for (i = 0; i < obj_size; i++)
    {
        PRINT("-----------");
    }
    PRINTLN("");

    PRINT_DISTANCE(11);
    for (i = 0; i < obj_size; i++)
    {
        PRINT_D("|", (int)objref->data[i]);
        PRINT_DISTANCE(11*(i+2));
    }
    PRINTLN("|");

    PRINT_DISTANCE(11);
    for (i = 0; i < obj_size; i++)
    {
        PRINT("-----------");
    }
    PRINTLN("");/*}}}*/
}

void printChar0(O obj)
{

    if (!obj->isArray)
        throwException("not array");
    short* p = (short*)obj->data;
    int i;
    for (i = 0; i < obj->length; i++)
    {
        char c = (char)(*p);
        printf("%c", c);
        p++;
    }
    printf("\n");


}

/**
  * print String Object.
  * for test
  */
void printString0(O obj)
{
    int value_offset;
    if (obj == NULL)
    {
        printf("obj is NULL!!!!");
        return;
    }

    //if (obj->isArray != 2)
    //throwException("not String");

    ClassBlock* cb = CLASS_CB(obj->class);
    FieldBlock* fb = (FieldBlock*)findField(java_lang_String, "value", "[C");

    if (fb == NULL)
        throwException("fb is null");

    value_offset = fb->offset;

    O c = OBJECT_DATA(obj, value_offset-1, O);

    printChar0(c);

}


void dumpClass(C class)
{
    ClassBlock* cb = CLASS_CB(class);
    printf("\ndumpClass>%s\n", cb->this_classname);
    printf("super class:%s\n", cb->super_classname);

    printf("Fileds count:%d\n", cb->fields_count);
    int i = 0;
    for (; i<cb->fields_count; i++)
    {
        FieldBlock* fb = &cb->fields[i];
        printf("%s|<%s>\n",fb->name, fb->type);
    }

    printf("Methods count:%d\n", cb->methods_count);
    i = 0;
    for (; i<cb->methods_count; i++)
    {
        MethodBlock* mb = &cb->methods[i];
        printf("%s|%s\n", mb->name, mb->type);
    }

    printf("Object size:%d\n", cb->obj_size);
}


#undef C
#undef O
#undef JF
#undef NF
