#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "command-line.h"
#include "control.h"
#include "../classloader/class.h"
#include "../lib/string.h"
#include "../debuger/log.h"
#include "../lib/trace.h"
#include "../lib/error.h"
#include "../lib/assert.h"
#include "../lib/mem.h"
#include "../lib/triple.h"
#include "../lib/poly.h"
#include "../lib/trace.h"

#define TRUE 1
#define FALSE 0

#define P Poly_t

static char *const VERSION = "turkey v0.0.7 linux/386";
static char *const WEBSITE = "https://github.com/qc1iu/turkeyVM";

static void actionArg_help();
static void actionArg_test();

typedef enum {
    ARGTYPE_BOOL,
    ARGTYPE_EMPTY,
    ARGTYPE_INT,
    ARGTYPE_STRING,
} Arg_type;

typedef struct {
    char *name;
    Arg_type type;
    char *arg;
    char *desc;
    void (*action) ();          //a call-back
} Arg_t;

static void arg_setTrace(char *func)
{
    Trace_addFunc(func);
}

static void arg_setLog(char *s)
{
    Log_add(s);
}

static void arg_setClassSearchPath(char *s)
{
    char *csp = String_concat(s, "/", NULL);
    setClassSearchPath(csp);
}

static void arg_setClassPath(char *s)
{
    char *classpath = String_concat(s, "/", NULL);
    setClassPath(classpath);
}

static void arg_setVerbose(int i)
{
    switch (i) {
    case 0:
        Control_verbose = VERBOSE_SILENT;
        break;
    case 1:
        Control_verbose = VERBOSE_PASS;
        break;
    case 2:
        Control_verbose = VERBOSE_SUBPASS;
        break;
    case 3:
        Control_verbose = VERBOSE_DETAIL;
        break;
    default:
        ERROR("-verbose {0|1|2|3}");
    }
}

static void arg_setOpcode()
{
    Control_opcode = 1;
}

static Arg_t allArgs[] = {
    {"cp",
     ARGTYPE_STRING,
     "{path}",
     "set class search path",
     arg_setClassSearchPath},
    {"classpath",
     ARGTYPE_STRING,
     "{path}",
     "set classpath",
     arg_setClassPath},
    {"opcode",
     ARGTYPE_EMPTY,
     "<NULL>",
     "statistic instrctions",
     arg_setOpcode},
    {"log",
     ARGTYPE_STRING,
     "{name}",
     "log method",
     arg_setLog},
    {"verbose",
     ARGTYPE_INT,
     "{0|1|2|3}",
     "verbose turkey",
     arg_setVerbose},
    {"trace",
     ARGTYPE_STRING,
     "{name}",
     "trace specific method",
     arg_setTrace},
    {"help",
     ARGTYPE_EMPTY,
     "<NULL>",
     "commandline list",
     actionArg_help},
    {"test",
     ARGTYPE_EMPTY,
     "<NULL>",
     "super test!!",
     actionArg_test},
    {NULL, 0, NULL, NULL, NULL}
};

static int printSpace(int i, int indent)
{
    Assert_ASSERT(i <= indent);
    while (i < indent) {
        i += fprintf(stdout, " ");
    }

    return i;
}

static void actionArg_test()
{
    dis_testinfo = TRUE;
}

static void printAllarg()
{
    int i = 0;
    for (; allArgs[i].action; i++) {
        int k = 0;
        k += fprintf(stdout, "  -%s", allArgs[i].name);
        k = printSpace(k, 15);
        k += fprintf(stdout, "%s", allArgs[i].arg);
        k = printSpace(k, 30);
        k += fprintf(stdout, "%s", allArgs[i].desc);
        fprintf(stdout, "\n");
    }
}

static int printUsage()
{
    fprintf(stdout,
            "Turkey is a Java virtual mechine for GNU Classpath0.0.6\n\n");
    fprintf(stdout, "Usage:\n\n");
    fprintf(stdout, "\tcommand [arguments]\n\n");
    fprintf(stdout, "The commands are:\n\n");
    printAllarg();
    fprintf(stdout, "\n");
    fprintf(stdout, "%s\n", VERSION);
    fprintf(stdout, "See %s for more details.\n", WEBSITE);
    fflush(stdout);
    return 0;
}

static void actionArg_help()
{
    printUsage();
    exit(0);
}

static void argException(char *fmt, ...)
{
    fprintf(stderr, "\e[31m\e[1mArgs error: \e[0m");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    fflush(stderr);
    exit(1);
}

/**
 * create a char**[] whitch contains all args of input file.
 * @parm index
 * @parm args
 */
static int createTurkeyArgs(int argc, char **argv, int index, char **args)
{
    int i;
    int j = 0;
    for (i = index; i < argc; i++, j++) {
        args[j] = String_new(argv[i]);
    }
    args[j] = NULL;

    return 0;
}

/**
 * figure up the arg count of input file.
 * @parm index the first arg's subscript of argv
 * @return input file's args count
 */
static int turkeyArgsCount(int argc, char **argv, int index)
{
    int i = index;
    int count = 0;
    while (i < argc) {
        count++;
        i++;
    }
    return count;
}

/**
 * print the triple for test.
 */
static void Trace_dor(Triple_t t)
{
    Assert_ASSERT(t);
    char *filename = (char *) Triple_first(t);
    char **args = (char **) Triple_second(t);
    int count = (int) Triple_third(t);

    fprintf(stdout, "filename:%s\n", filename);
    int i = 0;
    fprintf(stdout, "args:");
    for (i = 0; i < count; i++)
        fprintf(stdout, "%s ", args[i]);
    fprintf(stdout, "\n");
    fprintf(stdout, "args.lenth:%d\n", count);

    return;
}

Triple_t commandline_doarg(int argc, char **argv)
{
    char *filename = NULL;
    char **args = NULL;
    Triple_t t = NULL;
    int count;
    int index;

    if (argc == 1) {
        printUsage();
        exit(0);
    }
    count = 0;
    index = 1;
    for (; index < argc; index++) {
        int i = 0;
        if (argv[index][0] != '-') {
            if (filename == NULL) {
                /*
                 * If find a file, we treat all arg behind filename as
                 * inputfile's args.
                 */
                filename = String_new(argv[index]);
                index++;

                count = turkeyArgsCount(argc, argv, index);
                Mem_newSize(args, count + 1);
                createTurkeyArgs(argc, argv, index, args);
                t = Triple_new(filename, args, (P) count);

                //for test
                //Trace_dor(t);

                return t;
            } else {
                ERROR("impossible");
                argException("only one file");
            }
        }

        for (; allArgs[i].action; i++) {
            if (strcmp(allArgs[i].name, argv[index] + 1) != 0) {
                continue;
            }

            switch (allArgs[i].type) {
            case ARGTYPE_EMPTY:{
                    allArgs[i].action();
                    break;
                }
            case ARGTYPE_INT:{
                    index++;
                    if (index >= argc)
                        argException("expect <INT> arg");
                    char *arg = argv[index];
                    int n = atoi(arg);
                    allArgs[i].action(n);
                    break;
                }
            case ARGTYPE_STRING:{
                    index++;
                    if (index >= argc)
                        argException("expect <STRING> arg");
                    char *arg = argv[index];
                    allArgs[i].action(arg);
                    break;
                }
            default:
                ERROR("impossible arg");
            }
            break;
        }
        if (!allArgs[i].action) {
            argException("Unrecognized option: %s", argv[index]);
        }
    }

    if (t == NULL)
        argException("expect a file");

    return NULL;
}

#undef P
