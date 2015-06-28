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
#include "linkedlist.h"
#include "control.h"
#include "resolve.h"
#include "vm.h"

#define SCAN_SIG(p, D, S)			\
   p++;               /* skip start ( */	\
   while(*p != ')') {				\
       if((*p == 'J') || (*p == 'D')) {		\
          D;					\
          p++;					\
      } else {					\
          S;					\
          if(*p == '[')				\
              for(p++; *p == '['; p++);		\
          if(*p == 'L')				\
              while(*p++ != ';');		\
          else					\
              p++;				\
      }						\
   }						\
   p++;               /* skip end ) */

#define VA_DOUBLE(args, sp) *(u8*)sp = va_arg(args, u8);sp+=2
#define VA_SINGLE(args, sp) *sp = va_arg(args, u4);sp+=1

extern LinkedList* head;
extern Frame* current_frame;
extern NativeFrame* nframe;
static int frame_num = 1;
static int method_num;

/*
 * to generate an unique id for frame->id
 */
static int getNewId() {
    method_num++;
    return frame_num++;
}

/*
 * Create a Native method frame. The frame only have locals which
 * size is same to the current_frame->mb's locals.
 * NOTE:A native method is not nest, not recursive.So the frame is
 *      no need prev field.
 * @qcliu 2015/03/06
 */
static void createNativeFrame(MethodBlock* mb) {
    unsigned short n_locals = mb->args_count;
    int n_locals_idx;
    nframe->mb = mb;

    nframe->locals = (unsigned int*)sysMalloc(sizeof(int) * n_locals);
    nframe->class = mb->class;
    //set 0
    memset(nframe->locals, 0, sizeof(int) * n_locals);
    //copyArgs(Frame* frame, MethodBlock* mb)
    if (!(mb->access_flags & ACC_STATIC))//non-static
        n_locals_idx = n_locals;
    else
       n_locals_idx = n_locals - 1;

    for (; n_locals_idx >= 0; n_locals_idx--) {
    //NOTE: equals 0 also need copy
        memcpy(nframe->locals + n_locals_idx, current_frame->ostack, sizeof(int));
        /*NOTE: pop the stack*/
        *current_frame->ostack = 0;
        current_frame->ostack--;
    }
}
/*
 * The method is not pop. It just clear the native_frame. Since
 * in a native method , it impossible to invoke another methods
 * through VM.
 * @qcliu 2015/03/06
 */
static void popNativeFrame() {
    memset(nframe, 0, sizeof(NativeFrame));
}
/*
 * Create the frame for every method, and create ostack
 * and local-variable table. Then, copy args from old frame's
 * stack to the new frame's locals.
 * note: The non-static method need to copy objectref to the
 *       locals[0] additionally
 * @qcliu 2015/01/28
 */
static void createFrame(MethodBlock* mb, va_list jargs, void* ret) {
    unsigned short max_stack = mb->max_stack;/*{{{*/
    unsigned short max_locals = mb->max_locals;
    int args_count = mb->args_count;
    int locals_idx = 0;
    Class* class = mb->class;
    ClassBlock* cb = CLASS_CB(class);


    Frame* frame = (Frame*)sysMalloc(sizeof(Frame));
    frame->mb = mb;
    frame->cp = &cb->constant_pool;
    frame->pc = mb->code;
    frame->class = class;
    frame->locals = (unsigned int*)sysMalloc(
                sizeof(int) * (max_stack + max_locals));
    frame->ret = 0;
    //ret = &(frame->ret);
    //set 0
    memset(frame->locals, 0, sizeof(int)*(max_stack+max_locals));
    //point to the previous slot of aviliable
    frame->ostack = frame->locals + max_locals - 1;

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

    if (jargs == NULL) {
        //copyArgs(Frame* frame, MethodBlock* mb)
        if (!(mb->access_flags & ACC_STATIC))//non-static
          locals_idx = args_count;
        else
          locals_idx = args_count - 1;

        for (; locals_idx >= 0; locals_idx--) {
        //NOTE: equals 0 also need copy
            memcpy(frame->locals + locals_idx, current_frame->ostack, sizeof(int));
            /*NOTE: pop the stack*/
            *current_frame->ostack = 0;
            current_frame->ostack--;
        }
    }
    else {
        printf("jarg not null\n");
        unsigned int* sp = frame->locals;
        *sp = va_arg(jargs, u4);
        sp++;
        char* sig = mb->type;
        SCAN_SIG(sig, VA_DOUBLE(jargs, sp), VA_SINGLE(jargs, sp));
    }

    //point to the prev
    frame->prev = current_frame;
    current_frame = frame;

    frame->id = getNewId();

    if (dis_testinfo) {
        printf("\n%dnew Frame:---- %d, name:%s\n",method_num, frame->id, frame->mb->name);
    }

        /*}}}*/
}

/*
 * In this method, no need to deal with the return value,
 * relevant opreration in the executeJava() rely on specific
 * opcode.
 * invoke by: executeJava() in interp.c
 * @qcliu 2015/01/28
 */
void popFrame() {
    Frame* temp = current_frame;

    if (dis_testinfo) {
        printf("pop Frame: %d  ", temp->id);
    }
//test---------------------------------------------
    //ClassBlock* cb = CLASS_CB(temp->class);
    //printf("pop frame name:%s, class:%s\n", temp->mb->name, cb->this_classname);
//---------------------------------------------------------
    current_frame = current_frame->prev;
    //free(temp->locals);
    //free(temp);
    frame_num--;

}

/*
 * Create a new Frame, and copy args form old stack to new locals.
 * Then, execute the new method.
 */
void executeMethod(MethodBlock* mb, va_list jargs) {
    void* ret;
    //new Frame
    if (0 == strcmp(mb->name, "hashCode"))
      printf("hashCode");

    if (mb->native_invoker) {
        if (dis_testinfo) {
            printf("This is a native method!!!");
            printf("name:%s, type:%s\n", mb->name, mb->type);
        }
        createNativeFrame(mb);

        //TODO According to the method->type, need change the return type.@qcliu 2015/03/06
        ((void (*)())mb->native_invoker)();

        popNativeFrame();
    }
    else {
        createFrame(mb, jargs, ret);
        //executeJava
        executeJava();
    
        popFrame();
    }
}

/*
 * Static main is a special method.
 * It's args must be passing manually.
 * note: As to the <clinit>, it may be run earlier than static main.
 *       but <clint> is ACC_STATIC and ()V certainly, therefore, it
 *       can use executeMethod() instead of executeStaticMain().
 * @qcliu 2015/01/30
 */
void executeStaticMain(MethodBlock* mb) {
    unsigned short max_stack = mb->max_stack;/*{{{*/
    unsigned short max_locals = mb->max_locals;
    int args_count = mb->args_count;
    int i = 0;
    Class* class = mb->class;
    ClassBlock* cb = CLASS_CB(class);


    Frame* frame = (Frame*)sysMalloc(sizeof(Frame));
    frame->mb = mb;
    frame->cp = &cb->constant_pool;
    frame->pc = mb->code;
    frame->class = mb->class;
    frame->locals = (unsigned int*)sysMalloc(
                sizeof(int) * (max_stack + max_locals));
    frame->ret = 0;
    //set 0
    memset(frame->locals, 0, sizeof(int)*(max_stack+max_locals));
    //point to the previous slot of aviliable
    frame->ostack = frame->locals + max_locals - 1;

    /*no need copy args from old stack to new locals*/

    frame->prev = current_frame;
    current_frame = frame;

    frame->id = getNewId();

    if (dis_testinfo)
        printf("\nnew Frame: %d\n", frame->id);/*}}}*/

    //
    executeJava();
    popFrame();
}



//static void executeMethodVaList(Object* ob, Class* class, MethodBlock* mb, va_list jargs)
//{
//}

void executeMethodArgs(Class* class, MethodBlock* mb, ...) {
    va_list jargs;

    va_start(jargs, mb);
    executeMethod(mb, jargs);
    va_end(jargs);
    
}
