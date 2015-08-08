#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command-line.h"
#include "control.h"
#include "../lib/trace.h"
#include "../lib/error.h"
#include "../lib/assert.h"

#define TRUE 1
#define FALSE 0

static const char* VERSION = "turkey v0.0.1";

char** filename;
int file_length;


void actionArg_help();
void actionArg_printVtable();
void actionArg_printList();
void actionArg_printBytecode();
void actionArg_test();


typedef enum
{
    ARGTYPE_BOOL,
    ARGTYPE_EMPTY,
    ARGTYPE_INT,
    ARGTYPE_STRING,
}Arg_type;

typedef struct
{
    char* name;
    Arg_type type;
    char* arg;
    char* desc;
    void (*action)(); //a call-back
} Arg_t;

static void arg_setTrace(char* func)
{
    //printf("set trace:%s\n", func);
    Trace_addFunc(func);
}


static void arg_setVerbose(int i)
{
    switch (i)
    {
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


static Arg_t allArgs[] =
{
    {"verbose", ARGTYPE_INT, "{0|1|2|3}", "verbose turkey",  arg_setVerbose},
    {"trace", ARGTYPE_STRING, "{name}", "trace specific method", arg_setTrace},
    {"help", ARGTYPE_EMPTY,"<NULL>", "commandline list", actionArg_help},
    {"disv", ARGTYPE_EMPTY, "<NULL>", "display vtable", actionArg_printVtable},
    {"dish", ARGTYPE_EMPTY,  "<NULL>", "display list", actionArg_printList},
    {"disb", ARGTYPE_EMPTY, "<NULL>", "display bytecode", actionArg_printBytecode},
    {"test", ARGTYPE_EMPTY, "<NULL>", "super test!!", actionArg_test},
    {NULL, 0, NULL, NULL, NULL}
};

void actionArg_test()
{
    dis_testinfo = TRUE;
}

void actionArg_printBytecode()
{
    dis_bytecode = TRUE;
}

void actionArg_printList()
{
    dis_list = TRUE;
}

void actionArg_printVtable()
{
    dis_vtable = TRUE;
}

static int printSpace(int i, int indent)
{
    Assert_ASSERT(i<=indent);
    while(i < indent)
    {
        i += printf(" ");
    }

    return i;
}

void printAllarg()
{
    int i = 0;
    for(; allArgs[i].action; i++)
    {
        int k=0;
        k += printf("  -%s", allArgs[i].name);
        k = printSpace(k, 15);
        k += printf("%s", allArgs[i].arg);
        k = printSpace(k, 30);
        k += printf("%s", allArgs[i].desc);
        printf("\n");
    }
}

void actionArg_help()
{
    printAllarg();
    exit(0);
}


static int printUsage()
{
    printf("Turkey is a Java virtual mechine for GNU Classpath0.0.6\n\n");
    printf("Usage:\n\n");
    printf("\tcommand [arguments]\n\n");
    printf("The commands are:\n\n");
    printAllarg();
    printf("\n");
    printf("%s\n", VERSION);
    return 0;
}

int j=0;
void commandline_doarg(int argc,char** argv)
{
    filename = argv;
    file_length = 0;
    if (argc == 1)
    {
        printUsage();
        exit(0);
    }
    if(argc == 2)
    {
        dis_vtable = FALSE;
        dis_list = FALSE;
        dis_bytecode = FALSE;
        dis_testinfo = FALSE;
    }
    int index = 1;
    for(; index < argc; index++)
    {
        char* inputName = argv[index];

        int i = 0;
        if(inputName[0] != '-')
        {
            filename[j++] = argv[index];
            file_length++;
            continue;
        }

        for(; allArgs[i].action; i++)
        {
            if(strcmp(allArgs[i].name,inputName+1) != 0)
            {
                continue;
            }

            switch(allArgs[i].type)
            {
                case ARGTYPE_EMPTY:
                    {
                        allArgs[i].action();
                        break;
                    }
                case ARGTYPE_INT:
                    {
                        //FIXME need bound check
                        index++;
                        char* arg = argv[index++];
                        int n = atoi(arg);
                        allArgs[i].action(n);
                        break;
                    }
                case ARGTYPE_STRING:
                    {
                        //FIXME need bound check
                        index++;
                        char* arg = argv[index++];
                        allArgs[i].action(arg);
                        break;
                    }
                default:
                    ERROR("impossible arg");
            }
            break;
        }
        if(!allArgs[i].action)
        {
            ERROR("no arg match!!\n");
        }

    }
}
