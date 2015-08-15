/*------------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  vm.c                                   */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2014/12/31                             */
/*  DESCRIPTION           :  main for the javaVM                    */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 *
 *
 *
 *//*}}}*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "turkey.h"
#include "../interp/execute.h"
#include "../heapManager/alloc.h"
#include "../util/testvm.h"
#include "../util/exception.h"
#include "../util/jstring.h"
#include "../control/command-line.h"
#include "../classloader/resolve.h"
#include "../control/control.h"
#include "../classloader/class.h"
#include "../interp/stackmanager.h"
#include "../lib/list.h"
#include "../lib/hash.h"
#include "../lib/string.h"
#include "../lib/poly.h"
#include "../lib/error.h"
#include "../lib/assert.h"
#include "../lib/triple.h"
#include "../control/verbose.h"
#include <time.h>
#define TRUE 1
#define FALSE 0
#define C Class_t
#define O Object_t
#define P Poly_t

// global linkedlist that record the class already loaded

//List_t CList = NULL;
//List_t DList = NULL;
Hash_t CMap = NULL;
Hash_t DMap = NULL;
int vmsize = 0;

C java_lang_Class;
C java_lang_String;
C java_lang_VMClass;

int initable = FALSE;

static char prim[] =
{
    'B',
    'S',
    'I',
    'J',
    'C',
    'F',
    'D',
    'Z',
    ' '
};

void doKey(P key)
{
    printf("%s\n", (char*)key);
}

void doValue(P v)
{
    Assert_ASSERT(v);
    C c = (C)v;
    ClassBlock* cb = CLASS_CB(c);
    printf("%s\n", cb->this_classname);
}

static void keyDup(P x, P y)
{
    ERROR("dup key");
}


void exitVM()
{
    printf("\nVM exit\n\n");
    exit(0);
}

static void initClassPath()
{
    char* classpath = getClassPath();
    //printf("%s\n", classpath);
    parseClassPath(classpath);
}

static void initSystemClass()
{
    //init primClass
    int i;
    for (i=0; prim[i]!= ' '; i++)
        findPrimitiveClass(prim[i]);

    java_lang_VMClass = loadClass("java/lang/VMClass");
    java_lang_Class = loadClass("java/lang/Class");
    C system = loadClass("java/lang/System");
    C object = loadClass("java/lang/Object");
    //java_lang_String = loadClass("java/lang/String");

    initable = TRUE;

    initClass(object);
    initClass(system);
    initClass(java_lang_Class);
    initClass(java_lang_VMClass);




    if (java_lang_VMClass)
    {
        O obj = allocObject(java_lang_VMClass);

        obj->binding = object;
        object->class = obj;

        obj = allocObject(java_lang_VMClass);
        obj->binding = java_lang_Class;
        java_lang_VMClass->class = obj;
    }


}

static void initList()
{
    //CList = List_new();
    //DList = List_new();
    CMap = Hash_new((long(*)(P))String_hashCode
                    ,(Poly_tyEquals)String_equals
                    ,keyDup);
    DMap = Hash_new((long(*)(P))String_hashCode
                    ,(Poly_tyEquals)String_equals
                    ,keyDup);
}


static int initVM()
{
    initClassPath();
    initList();
    initFrame();
    initNativeFrame();
    initSystemClass();
    return 0;
}


int main(int argc, char** argv)
{
    /*{{{*/
    clock_t start, end;
    Triple_t t;
    char* filename;
    char** _args;
    C main_class;
    C stringArray;
    MethodBlock* main_mehtod;
    O args;
    int args_size;
    
    start = clock();
    t = commandline_doarg(argc, argv);
    Assert_ASSERT(t);
    filename = (char*)Triple_first(t);
    _args = (char**)Triple_second(t);
    args_size = (int)Triple_third(t);

    int r;
    Verbose_TRACE("initVM", initVM, (), r, VERBOSE_PASS);

    main_class = loadClass(filename);
    Assert_ASSERT(main_class);

    //find access main method!
    main_mehtod = findMethod(main_class, "main", "([Ljava/lang/String;)V");

    if ((!main_mehtod)||!(main_mehtod->access_flags & ACC_STATIC))
        throwException("Not find static main");

    stringArray= NULL;
    stringArray = findArrayClass("[Ljava/lang/String;");
    Assert_ASSERT(stringArray);
    args = allocArray(stringArray, args_size, sizeof(O), TYPE_REFERENCE);
    int i;
    for (i = 0; i<args_size; i++)
    {
        ARRAY_DATA(args, i, O) = createJstring(_args[i]);
    }

    executeStaticMain(main_mehtod, args);

    end = clock();

    printf("\nVM run %f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
    //Hash_foreachKey(CMap, doKey);
    Hash_status(CMap);
    return 0; 
    
    /*}}}*/
}



char* getMethodClassName(MethodBlock* mb)
{
    Assert_ASSERT(mb);
    ClassBlock* cb = CLASS_CB(mb->class);
    return cb->this_classname;
}


#undef C
#undef O
#undef P
