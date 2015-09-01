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
#include "../interp/opcode.h"
#include "../dll/dll.h"
#include "../lib/list.h"
#include "../lib/hash.h"
#include "../lib/string.h"
#include "../lib/poly.h"
#include "../lib/error.h"
#include "../lib/assert.h"
#include "../lib/triple.h"
#include "../control/verbose.h"
#include <time.h>

#define C Class_t
#define O Object_t
#define P Poly_t

int vmsize = 0;

void doKey(P key)
{
    printf("%s\n", (char *) key);
}

void doValue(P v)
{
    Assert_ASSERT(v);
    C c = (C) v;
    ClassBlock_t *cb = CLASS_CB(c);
    printf("%s\n", cb->this_classname);
}

void exitVM()
{
    printf("\nVM exit\n\n");
    exit(0);
}

static void initClassPath()
{
    char *classpath = getClassPath();
    parseClassPath(classpath);
}

static void initHashMap()
{
    initDllHash();
    initClassHash();
}

static int initVM()
{
    initClassPath();
    initHashMap();
    initFrame();
    initNativeFrame();
    initSystemClass();
    return 0;
}

int main(int argc, char **argv)
{
    /*{{{ */
    clock_t start, end;
    Triple_t t;
    char *filename;
    char **_args;
    C main_class;
    C stringArray;
    MethodBlock_t *main_mehtod;
    O args;
    int args_size;

    start = clock();
    t = commandline_doarg(argc, argv);
    Assert_ASSERT(t);
    filename = (char *) Triple_first(t);
    _args = (char **) Triple_second(t);
    args_size = (int) Triple_third(t);

    int r;
    Verbose_TRACE("initVM", initVM, (), r, VERBOSE_PASS);
    main_class = loadClass(filename);
    Assert_ASSERT(main_class);
    //find access main method!
    main_mehtod = findMethod(main_class, "main", "([Ljava/lang/String;)V");

    if ((!main_mehtod) || !(main_mehtod->access_flags & ACC_STATIC))
        throwException("Not find static main");

    stringArray = NULL;
    stringArray = findArrayClass("[Ljava/lang/String;");
    Assert_ASSERT(stringArray);
    args = allocArray(stringArray, args_size, sizeof(O), TYPE_REFERENCE);
    int i;
    for (i = 0; i < args_size; i++) {
        ARRAY_DATA(args, i, O) = createJstring(_args[i]);
    }

    executeStaticMain(main_mehtod, args);

    end = clock();
    printf("\nVM run %f seconds\n", (double) (end - start) / CLOCKS_PER_SEC);
    //Hash_foreachKey(CMap, doKey);
    classHashStatus();
    opcodeStatus();

    return 0;
    /*}}} */
}

char *getMethodClassName(MethodBlock_t * mb)
{
    Assert_ASSERT(mb);
    ClassBlock_t *cb = CLASS_CB(mb->class);
    return cb->this_classname;
}

#undef C
#undef O
#undef P
