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
#include "../control/command-line.h"
#include "../classloader/resolve.h"
#include "../control/control.h"
#include "../classloader/class.h"
#include "../util/exception.h"
#include "../interp/stackmanager.h"
#include "../lib/list.h"
#include "../lib/hash.h"
#include "../lib/string.h"
#include "../lib/poly.h"
#include "../lib/error.h"
#include "../lib/assert.h"
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


static void initVM()
{
    initClassPath();
    initList();
    initFrame();
    initNativeFrame();
    initSystemClass();
}

int main(int argc, char** argv)
{
    clock_t start, end;/*{{{*/
    start = clock();
    commandline_doarg(argc, argv);

    int i = 0;
    C class=NULL;
    if (argc<2)
    {
    }

    initVM();
    /*
#define NRETURN
    Verbose_TRACE("initVM", initVM, ((void)), r, VERBOSE_PASS);
#undef NRETURN
*/

    //class = loadClass("[[[[LHello;");
    parseFilename(argv[1]);
    class = loadClass(argv[1]);

    //find access main method!
    MethodBlock* main = findMethod(class, "main", "([Ljava/lang/String;)V");
    if ((!main)||!(main->access_flags & ACC_STATIC))
    {
        printf("not found static main!\n");
        exit(0);
    }
    else
    {
        if (dis_testinfo)
            printf("find static main!\n");
    }

    printf("Init VM Ok...\n");
    executeStaticMain(main);

    if (dis_testinfo)
        printf("vmsize : %d B\n", vmsize);
    //exitVM();




    if (dis_list)
        ERROR("TODO");
    //printList(head);
    //  printVtable(head);

    end = clock();

    printf("\nVM run %f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
    //Hash_foreachKey(CMap, doKey);
    //Hash_status(CMap);
    return 0; /*}}}*/
}

#undef C
#undef O
#undef P