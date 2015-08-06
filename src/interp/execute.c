/*------------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  execute.c                              */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2015/01/23                             */
/*  DESCRIPTION           :  for turkeyVM                           */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 *
 *
 *
 *//*}}}*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stackmanager.h"
#include "../interp/interp.h"
#include "../heapManager/alloc.h"
#include "../lib/list.h"
#include "../control/control.h"
#include "../classloader/resolve.h"
#include "../main/turkey.h"
#include "../util/exception.h"
#include "../lib/trace.h"
#include "../util/testvm.h"

#define C Class_t
#define O Object_t
#define JF JFrame_t


extern List_t CList;




/*
 * Create a new Frame, and copy args form old stack to new locals.
 * Then, execute the new method.
 */
void executeMethod(MethodBlock* mb, va_list jargs)
{
    void* ret;/*{{{*/
    if (mb->native_invoker)
    {
        if (dis_testinfo)
        {
            printf("This is a native method!!!");
            printf("name:%s, type:%s\n", mb->name, mb->type);
        }
        createNativeFrame(mb);

        //TODO According to the method->type, need change the return type.@qcliu 2015/03/06
        ((void (*)())mb->native_invoker)();

        popNativeFrame();
        //printf("pop Native method:%s\n", mb->name);
    }
    else
    {
        JF retFrame = getCurrentFrame();

        createFrame(mb, jargs, ret);
        //executeJava
        Trace_Stack(mb->name, executeJava, (retFrame), printStack);
        //executeJava(retFrame);

        popFrame();
    }
    /*}}}*/
}

/*
 * Static main is a special method.
 * It's args must be passing manually.
 * note: As to the <clinit>, it may be run earlier than static main.
 *       but <clint> is ACC_STATIC and ()V certainly, therefore, it
 *       can use executeMethod() instead of executeStaticMain().
 * @qcliu 2015/01/30
 */
void executeStaticMain(MethodBlock* mb)
{
    unsigned short max_stack = mb->max_stack;/*{{{*/
    unsigned short max_locals = mb->max_locals;
    int args_count = mb->args_count;
    JF frame = createFrame0(mb);

    if (dis_testinfo)
        printf("\nnew Frame: %d\n", getCurrentFrameId());
    //
    executeJava(NULL);
    popFrame();
    /*}}}*/
}



void executeMethodArgs(C class, MethodBlock* mb, ...)
{
    va_list jargs;

    va_start(jargs, mb);
    executeMethod(mb, jargs);
    va_end(jargs);
}

/**
 * @see native.c
 */
void invoke(MethodBlock* mb, O args, O this)
{
    /*{{{*/
    int args_count = mb->args_count;
    int locals_idx = 0;
    JF retFrame = getCurrentFrame();
    JF frame = createFrame0(mb);
    //copy args
    /*
     * old_stack                 new_locals
     * --------                   ---------
     * |objref|                [0]|       |
     * --------                   ---------
     * |arg1  |                [1]|       |
     * --------                   ---------<-locals_idx(static)
     * |arg2  |                [2]|       |
     * --------<-top              ---------<-locals_idx(non-static)
     *
     * @qcliu 2015/01/29
     */

    if (args->isArray != 1)
    {
        throwException("args must be array");
    }
    //copyArgs(Frame* frame, MethodBlock* mb)
    if (!(mb->access_flags & ACC_STATIC))//non-static
        locals_idx = args_count;
    else
        throwException("construct method must be non-static");

    if (args->length != locals_idx)
    {
        throwException("args count error");
    }

    //*((Object**)&frame->locals[0]) = this;
    store(&this, TYPE_REFERENCE, 0);
    int i;
    for (i = 0; i<args->length; i++)
    {
        //*((Object**)&frame->locals[i+1]) = ARRAY_DATA(args, i, Object*);
        store(&(ARRAY_DATA(args, i, O)), TYPE_REFERENCE, i+1);

        //NOTE: equals 0 also need copy
        //memcpy(frame->locals + locals_idx, current_frame->ostack, sizeof(int));
        /*NOTE: pop the stack*/
        //*current_frame->ostack = 0;
        //current_frame->ostack--;
    }

    executeJava(retFrame);

    popFrame();
    /*}}}*/
}

#undef C
#undef O
#undef JF
