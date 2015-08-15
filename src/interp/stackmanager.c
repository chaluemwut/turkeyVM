/*^_^*--------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  stackmanager.c                         */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2015/07/15                             */
/*  DESCRIPTION           :  stack relevent op                      */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 *
 *//*}}}*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../main/turkey.h"
#include "../heapManager/alloc.h"
#include "../control/control.h"
#include "stackmanager.h"
#include "../util/testvm.h"
#include "../lib/stack.h"
#include "../lib/error.h"
#include "../lib/mem.h"
#include "../lib/trace.h"


#define C Class_t
#define O Object_t
#define JF JFrame_t
#define NF NFrame_t

static JF current_frame;
static NF nframe;
static unsigned int* bottom;
static unsigned int* top;
static int frame_num = 1;
static int method_num;

/**
 * method call stack
 * @see createFrame0() popFrame()
 */
Stack_t methodF = NULL;

//TODO useless
Stack_t nativeF = NULL;

/**
 * to generate an unique id for frame->id
 */
int getNewId()
{
    method_num++;
    return frame_num++;
}

/**
 * NOTE: need a dummy node otherwise popFrame()
 *      will error.
 */
JF initFrame()
{
    methodF = Stack_new();
    nativeF = Stack_new();

    JF f;
    Mem_new(f);
    f->mb = NULL;
    f->ostack=NULL;
    f->locals=NULL;

    Stack_push(methodF, f);

    return NULL;
}

void initNativeFrame()
{
    NF frame = (NF)sysMalloc(sizeof(struct NF));
    frame->mb = NULL;
    frame->class = NULL;
    frame->locals = NULL;
    nframe = frame;

}

/*
 * Create a Native method frame. The frame only have locals which
 * size is same to the current_frame->mb's locals.
 * NOTE:A native method is not nest, not recursive.So the frame is
 *      no need prev field.
 * @qcliu 2015/03/06
 */
void createNativeFrame(MethodBlock* mb)
{
    //TODO args_count??? constrcutNative 3 or 4??
    //printf("Native method:%s\n", mb->name);
    unsigned short n_locals = mb->args_count;

    /*@TEST
     *
     * 2015/07/06
     * */
    if (!(mb->access_flags & ACC_STATIC))
        n_locals++;

    int n_locals_idx;
    nframe->mb = mb;

    nframe->locals = (unsigned int*)sysMalloc(sizeof(int) * n_locals);
    nframe->class = mb->class;
    //set 0
    memset(nframe->locals, 0, sizeof(int) * n_locals);
    //copyArgs(Frame* frame, MethodBlock* mb)
    if (!(mb->access_flags & ACC_STATIC))//non-static
        n_locals_idx = n_locals-1;
    else
        n_locals_idx = n_locals - 1;

    for (; n_locals_idx >= 0; n_locals_idx--)
    {
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
void popNativeFrame()
{
    memset(nframe, 0, sizeof(struct NF));
}

/**
 * @parm mb
 *      refer to the size of stack and locals
 *
 * @return Creating a JF according to the Methodblock,
 *      no need to copy args. just change methodF and
 *      current_frame.
 */
JF createFrame0(MethodBlock* mb)
{
    unsigned short max_stack = mb->max_stack;
    unsigned short max_locals = mb->max_locals;

    int args_count = mb->args_count;
    int locals_idx = 0;
    C class = mb->class;
    ClassBlock* cb = CLASS_CB(class);

    JF frame = (JF)sysMalloc(sizeof(struct JF));
    frame->mb = mb;
    frame->cp = &cb->constant_pool;
    frame->pc = mb->code;
    frame->class = class;
    frame->locals = (unsigned int*)sysMalloc(
                        sizeof(int) * (max_stack + max_locals));
    frame->ret = 0;
    frame->pc_offset = 0;
    //ret = &(frame->ret);
    //set 0
    memset(frame->locals, 0, sizeof(int)*(max_stack+max_locals));
    //point to the previous slot of aviliable
    frame->ostack = frame->locals + max_locals - 1;

    frame->bottom = frame->ostack;
    frame->top = frame->ostack+max_stack;
    frame->id = getNewId();

    //point to the prev
    Stack_push(methodF, frame);
    setCurrentFrame(frame);
    bottom = current_frame->bottom;
    top = current_frame->top;



    return frame;

}

/*
 * Create the frame for every method, and create ostack
 * and local-variable table. Then, copy args from old frame's
 * stack to the new frame's locals.
 * note: The non-static method need to copy objectref to the
 *       locals[0] additionally
 * @qcliu 2015/01/28
 *
 *    //copy args
      old_stack                 new_locals
      --------                   ---------
      |objref|                [0]|       |
      --------                   ---------
      |arg1  |                [1]|       |
      --------                   ---------<-locals_idx(static)
      |arg2  |                [2]|       |
      --------<-top              ---------<-locals_idx(non-static)

   @qcliu 2015/01/29

 */
JF createFrame(MethodBlock* mb, va_list jargs, void* ret)
{
    JF prev = getCurrentFrame();
    JF frame = createFrame0(mb);

    int args_count = mb->args_count;
    int locals_idx = 0;


    //XXX copy args
    if (jargs == NULL)
    {
        //copyArgs(Frame* frame, MethodBlock* mb)
        if (!(mb->access_flags & ACC_STATIC))//non-static
            locals_idx = args_count;
        else
            locals_idx = args_count - 1;

        for (; locals_idx >= 0; locals_idx--)
        {
            //NOTE: equals 0 also need copy
            memcpy(frame->locals + locals_idx, prev->ostack, sizeof(int));
            /*NOTE: pop the stack*/
            *prev->ostack = 0;
            prev->ostack--;
        }
    }
    else
    {
        //printf("jarg not null\n");
        unsigned int* sp = frame->locals;
        if (!(mb->access_flags & ACC_STATIC))//non-static
        {
            *sp = va_arg(jargs, u4);
            sp++;
        }
        char* sig = mb->type;
        SCAN_SIG(sig, VA_DOUBLE(jargs, sp), VA_SINGLE(jargs, sp));
    }

    if (dis_testinfo)
    {
        printf("\n%dnew Frame:---- %d, name:%s\n",method_num, frame->id, frame->mb->name);
    }

    return frame;
}

/*
 * In this method, no need to deal with the return value,
 * relevant opreration in the executeJava() rely on specific
 * opcode.
 * invoke by: executeJava() in interp.c
 * @qcliu 2015/01/28
 */
JF popFrame()
{
    JF temp = (JF)Stack_pop(methodF);
    //printf("pop frame:%s, size:%d\n", temp->mb->name, Stack_size(methodF));

    if (dis_testinfo)
    {
        printf("pop Frame: %d  ", temp->id);
    }
    free(temp->locals);
    free(temp);
    frame_num--;

    //XXX If not have a dummy node, will error.
    JF f = (JF)Stack_seek(methodF);
    if (f == NULL)
        ERROR("stack is NULL");

    setCurrentFrame(f);

    bottom = current_frame->bottom;
    top = current_frame->top;

    return f;

}

JF getCurrentFrame()
{
    return current_frame;
}


void setCurrentFrame(JF f)
{
    current_frame = f;
}


void PCIncrease(int x)
{
    current_frame->pc+=x;
    current_frame->pc_offset+=x;
}

void PCDecrease(int x)
{
    current_frame->pc-=x;
    current_frame->pc_offset-=x;
}

NF getNativeFrame()
{
    return nframe;
}



#define ASSERT_STACK(f)                                 \
    do {                                                \
        if (assert_stack)                               \
        {                                               \
            if (f->ostack<f->bottom                     \
                        ||f->ostack>f->top)             \
            {                                           \
                printStack(f);                          \
                    ERROR("stack error");               \
            }                                           \
        }                                               \
    }while(0)


void load(void* result, Operand_Type t, int index)
{
    switch (t)
    {
    case TYPE_INT:
        *(int*)result = *(int*)(current_frame->locals+index);
        break;
    case TYPE_CHAR:
        *(char*)result = *(char*)(current_frame->locals+index);
        break;
    case TYPE_LONG:
        *(long long*)result = *(long long*)(current_frame->locals+index);
        break;
    case TYPE_UINT:
        *(unsigned int*)result = *(unsigned int*)(current_frame->locals+index);
        break;
    case TYPE_FLOAT:
        *(float*)result = *(float*)(current_frame->locals+index);
        break;
    case TYPE_DOUBLE:
        *(double*)result = *(double*)(current_frame->locals+index);
        break;
    case TYPE_REFERENCE:
        *(O*)result = *(O*)(current_frame->locals+index);
        break;
    default:
        ERROR("wrong type");
    }
}

void store(void* value, Operand_Type t, int index)
{
    switch (t)
    {
    case TYPE_INT:
        *(int*)(current_frame->locals+index) = *(int*)value;
        break;
    case TYPE_CHAR:
        *(char*)(current_frame->locals+index) = *(char*)value;
        break;
    case TYPE_LONG:
        *(long long*)(current_frame->locals+index) = *(long long*)value;
        break;
    case TYPE_UINT:
        *(unsigned int*)(current_frame->locals+index) = *(unsigned int*)value;
        break;
    case TYPE_FLOAT:
        *(float*)(current_frame->locals+index) = *(float*)value;
        break;
    case TYPE_DOUBLE:
        *(double*)(current_frame->locals+index) = *(double*)value;
        break;
    case TYPE_REFERENCE:
        *(O*)(current_frame->locals+index) = *(O*)value;
        break;
    default:
        ERROR("wrong type");
    }
}

void seek(void* result, Operand_Type t)
{
    TODO("seek");
}

void pop(JF f, void* result, Operand_Type t)
{
    switch (t)
    {
    case TYPE_INT:
        *(int*)result = *(int*)f->ostack;
        *(int*)f->ostack = 0;
        f->ostack--;
        ASSERT_STACK(f);
        break;
    case TYPE_FLOAT:
        *(float*)result = *(float*)f->ostack;
        *(int*)f->ostack = 0;
        f->ostack--;
        ASSERT_STACK(f);
        break;
    case TYPE_LONG:
        f->ostack--;
        *(long long*)result = *(long long*)f->ostack;
        *(long long*)f->ostack = 0;
        //*(int*)current_frame->ostack = 0;
        f->ostack--;
        break;
    case TYPE_ULONG:
        f->ostack--;
        *(unsigned long long*)result = *(unsigned long long*)f->ostack;
        *(long long*)f->ostack = 0;
        //*(int*)current_frame->ostack = 0;
        f->ostack--;
        break;
    case TYPE_DOUBLE:
        f->ostack--;
        *(double*)result = *(double*)f->ostack;
        *(long long*)f->ostack = 0;
        //*(int*)current_frame->ostack = 0;
        f->ostack--;
        ASSERT_STACK(f);
        break;
    case TYPE_REFERENCE:
        *(O*)result = *(O*)f->ostack;
        *(int*)f->ostack = 0;
        f->ostack--;
        ASSERT_STACK(f);
        break;
    case TYPE_CHAR:
        *(char*)result = *(char*)f->ostack;
        *(int*)f->ostack = 0;
        f->ostack--;
        ASSERT_STACK(f);
        break;
    case TYPE_UINT:
        *(unsigned int*)result = *(unsigned int*)f->ostack;
        *(int*)f->ostack = 0;
        f->ostack--;
        ASSERT_STACK(f);
        break;
    default:
        ERROR("wrong type");
    }
}


/**
 * push value to the given frame's stack
 *
 * @parm frame
 *      the frame push value to.
 *
 */
void push(JF frame, void* value, Operand_Type type)
{
    frame->ostack++;
    ASSERT_STACK(frame);
    switch (type)
    {
    case TYPE_INT:
        *(int*)(frame->ostack) = *(int*)value;
        break;
    case TYPE_FLOAT:
        *(float*)(frame->ostack) = *(float*)value;
        break;
    case TYPE_DOUBLE:
        *(double*)(frame->ostack) = *(double*)value;
        frame->ostack++;
        break;
    case TYPE_LONG:
        *(long long*)(frame->ostack) = *(long long*)value;
        frame->ostack++;
        break;
    case TYPE_CHAR:
        *(char*)(frame->ostack) = *(char*)value;
        break;
    case TYPE_REFERENCE:
        *(O*)(frame->ostack) = *(O*)value;
        break;
    case TYPE_UINT:
        *(unsigned int*)(frame->ostack) = *(unsigned int*)value;
        break;
    default:
        ERROR("wrong type");
    }
}


#undef C
#undef O
#undef JF
#undef NF
