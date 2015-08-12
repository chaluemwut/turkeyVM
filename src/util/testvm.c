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
#include "../lib/assert.h"

/*packing the print operation*/
/*{{{*/
#define PRINT_DISTANCE(fd, d)    \
    if(distance>d)                 \
        throwException("IndentError");          \
    for(;distance < d;distance++)  \
    {                               \
        fprintf(fd, "%s", " ");                 \
    }

#define PRINT_SPACE(fd)             \
        t = indentLevel;    \
        for(;t > 0;t--)          \
          distance += fprintf(fd, "%s", " ")
//"..."&...
#define PRINT_D(fd,d,s)        distance += fprintf(fd, d"%d",s)
#define PRINT_DS(fd,d,s)       PRINT_SPACE;distance += fprintf(fd,d"%d",s)
#define PRINT_X(fd,d,s)        distance += fprintf(fd,d"%x",s)
#define PRINT_XS(fd,d,s)       PRINT_SPACE;distance += fprintf(fd,d"%x",s)
#define PRINT_S(fd,d,s)        distance += fprintf(fd,d"%s",s)
#define PRINT_SS(fd,d,s)       PRINT_SPACE;distance += fprintf(fd,d"%s",s)
//"..."
#define PRINT_TAB(fd)           distance = fprintf(fd,"    ")
#define PRINT(fd,s)            distance += fprintf(fd,s)
#define PRINT_SP(fd,s)         PRINT_SPACE;distance += fprintf(fd,s)
//clear the distance
#define PRINTLN(fd,s)          distance = 0;fprintf(fd,s"\n")
#define PRINTLN_S(fd,s)        PRINT_SPACE;distance = 0;fprintf(fd,s"\n")
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

    PRINTLN(stdout,"");
    PRINT_DISTANCE(stdout,10);
    PRINTLN(stdout,"stack");
    for (; max_stack > 0; max_stack--)
    {
        if (postack == current_frame->ostack)
        {
            PRINT_DISTANCE(stdout,5);
            PRINT(stdout,"top->");
        }
        PRINT_DISTANCE(stdout,10);
        PRINTLN(stdout,"----------------");
        PRINT_DISTANCE(stdout,10);
        PRINT_D(stdout,"|", *(int*)postack);
        PRINT_DISTANCE(stdout,25);
        PRINTLN(stdout,"|");
        postack--;
    }

    PRINT_DISTANCE(stdout,4);
    PRINT(stdout,"bottom");
    PRINTLN(stdout,"----------------");

    /*print locals*/
    PRINTLN(stdout,"");
    PRINT_DISTANCE(stdout,10);
    PRINTLN(stdout,"locals");
    postack = current_frame->locals;
    for (locals_idx = 0; max_locals > 0; max_locals--, locals_idx++)
    {
        PRINT_DISTANCE(stdout,10);
        PRINTLN(stdout,"----------------");
        PRINT_DISTANCE(stdout,7);
        PRINT(stdout,"[");
        PRINT_D(stdout,"", locals_idx);
        PRINT(stdout,"]");
        PRINT_D(stdout,"|", *(int*)postack);
        PRINT_DISTANCE(stdout,25);
        PRINTLN(stdout,"|");
        postack++;
    }
    PRINT_DISTANCE(stdout,10);
    PRINTLN(stdout,"----------------");
    /*}}}*/
}









void printStackLog(FILE* fd, JF current_frame)
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

    PRINTLN(fd,"");
    PRINT_DISTANCE(fd,10);
    PRINTLN(fd,"stack");
    for (; max_stack > 0; max_stack--)
    {
        if (postack == current_frame->ostack)
        {
            PRINT_DISTANCE(fd,5);
            PRINT(fd,"top->");
        }
        PRINT_DISTANCE(fd,10);
        PRINTLN(fd,"----------------");
        PRINT_DISTANCE(fd,10);
        PRINT_D(fd,"|", *(int*)postack);
        PRINT_DISTANCE(fd,25);
        PRINTLN(fd,"|");
        postack--;
    }

    PRINT_DISTANCE(fd,4);
    PRINT(fd,"bottom");
    PRINTLN(fd,"----------------");

    /*print locals*/
    PRINTLN(fd,"");
    PRINT_DISTANCE(fd,10);
    PRINTLN(fd,"locals");
    postack = current_frame->locals;
    for (locals_idx = 0; max_locals > 0; max_locals--, locals_idx++)
    {
        PRINT_DISTANCE(fd,10);
        PRINTLN(fd,"----------------");
        PRINT_DISTANCE(fd,7);
        PRINT(fd,"[");
        PRINT_D(fd,"", locals_idx);
        PRINT(fd,"]");
        PRINT_D(fd,"|", *(int*)postack);
        PRINT_DISTANCE(fd,25);
        PRINTLN(fd,"|");
        postack++;
    }
    PRINT_DISTANCE(fd,10);
    PRINTLN(fd,"----------------");
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

    if (objref->type == TYPE_ARRAY)
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

    PRINTLN(stdout,"");
    PRINT_DISTANCE(stdout,11);
    PRINT_S(stdout,"",classname);
    PRINTLN(stdout,"");

    PRINT_DISTANCE(stdout,11);
    for (i = 0; i < obj_size; i++)
    {
        PRINT(stdout,"-----------");
    }
    PRINTLN(stdout,"");

    PRINT_DISTANCE(stdout,11);
    for (i = 0; i < obj_size; i++)
    {
        PRINT_D(stdout,"|", (int)objref->data[i]);
        PRINT_DISTANCE(stdout,11*(i+2));
    }
    PRINTLN(stdout,"|");

    PRINT_DISTANCE(stdout,11);
    for (i = 0; i < obj_size; i++)
    {
        PRINT(stdout,"-----------");
    }
    PRINTLN(stdout,"");/*}}}*/
}

void printChar0(O obj)
{
    Assert_ASSERT(obj);
    Assert_ASSERT(obj->type == TYPE_ARRAY);
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



#undef C
#undef O
#undef JF
#undef NF
