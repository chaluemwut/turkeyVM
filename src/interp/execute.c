#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stackmanager.h"
#include "../interp/interp.h"
#include "../heapManager/alloc.h"
#include "../debuger/log.h"
#include "../control/control.h"
#include "../classloader/resolve.h"
#include "../main/turkey.h"
#include "../util/exception.h"
#include "../util/testvm.h"
#include "../control/verbose.h"
#include "../lib/string.h"
#include "../lib/assert.h"
#include "../lib/list.h"
#include "../lib/trace.h"
#include "../lib/error.h"

#define C Class_t
#define O Object_t
#define JF JFrame_t

extern List_t CList;

static int executeNativeMethod(MethodBlock_t * mb)
{
    if (dis_testinfo) {
        printf("This is a native method!!!");
        printf("name:%s, type:%s\n", mb->name, mb->type);
    }
    createNativeFrame(mb);
    JF retFrame = getCurrentFrame();
    //TODO According to the method->type, need change the return type.@qcliu 2015/03/06
    ((void (*)(JF)) mb->native_invoker) (retFrame);
    popNativeFrame();
    //printf("pop Native method:%s\n", mb->name);
    return 0;
}

int Log_executeJava(JF retFrame, JF currentF)
{
    executeJava(retFrame, currentF);
    popFrame();
    return 0;
}

/*
 * Create a new Frame, and copy args form old stack to new locals.
 * Then, execute the new method.
 */
int Verbose_executeMethod(MethodBlock_t * mb, va_list jargs)
{
    void *ret;                  /*{{{ */
    if (mb->native_invoker) {
        return executeNativeMethod(mb);
    } else {
        JF retFrame = getCurrentFrame();
        createFrame(mb, jargs, ret);
        JF currentF = getCurrentFrame();
        //executeJava

        char *s = String_concat(getMethodClassName(mb), ":", mb->name, mb->type,
                                NULL);
        int r;
        Log_SingleMethod(s, Log_executeJava, (retFrame, currentF), r);
    }
    return 0;
    /*}}} */
}

void executeMethod(MethodBlock_t * mb, va_list jargs)
{
    char *cName = getMethodClassName(mb);
    char *mName = mb->name;
    char *info = String_concat(cName, ":", mName, mb->type, NULL);
    int i;

    Verbose_TRACE(info, Verbose_executeMethod, (mb, jargs), i, VERBOSE_DETAIL);
}

/*
 * Static main is a special method.
 * It's args must be passing manually.
 * note: As to the <clinit>, it may be run earlier than static main.
 *       but <clint> is ACC_STATIC and ()V certainly, therefore, it
 *       can use executeMethod() instead of executeStaticMain().
 * @qcliu 2015/01/30
 */
static int Verbose_executeStaticMain(MethodBlock_t * mb, O args)
{
    unsigned short max_stack = mb->max_stack; /*{{{ */
    unsigned short max_locals = mb->max_locals;
    int args_count = mb->args_count;
    JF frame = createFrame0(mb);
    *(O *) (frame->locals + 0) = *(O *) & args;
    if (dis_testinfo)
        printf("\nnew Frame: %d\n", frame->id);
    int r;
    Log_SingleMethod("staticMain", Log_executeJava, (NULL, frame), r);
    return 0;
    /*}}} */
}

void executeStaticMain(MethodBlock_t * mb, O args)
{
    int i;
    Verbose_TRACE("static Main", Verbose_executeStaticMain, (mb, args), i,
                  VERBOSE_PASS);
}

void executeMethodArgs(C class, MethodBlock_t * mb, ...)
{
    va_list jargs;

    va_start(jargs, mb);
    executeMethod(mb, jargs);
    va_end(jargs);
}

/**
 * @see native.c
 */
void invokeConstructNative(MethodBlock_t * mb, O args, O this)
{
    /*{{{ */
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

    Assert_ASSERT(args->type == OBJECT_ARRAY);
    if (!(mb->access_flags & ACC_STATIC)) //non-static
        locals_idx = args_count;
    else
        throwException("construct method must be non-static");

    if (args->length != locals_idx) {
        throwException("args count error");
    }

    STORE(frame, this, O, 0);
    int i;
    for (i = 0; i < args->length; i++) {
        STORE(frame, (ARRAY_DATA(args, i, O)), O, i + 1);
        /*
         * Since this method is not normal, it's not have a prev frame,
         * we don't need to copy args for it, just create it's locals
         * manually.
         */
    }
    executeJava(retFrame, frame);
    popFrame();
    /*}}} */
}

#undef C
#undef O
#undef JF
