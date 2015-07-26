#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Command-line.h"
#include "control.h"
#include "../lib/trace.h"

#define TRUE 1
#define FALSE 0

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
    char* desc;
    void (*action)(); //a call-back
} Arg_t;

static void arg_setTrace(char* func)
{
    //printf("set trace:%s\n", func);
    Trace_addFunc(func);
}

static Arg_t allArgs[] =
{
    {"trace", ARGTYPE_STRING, "{name}", arg_setTrace},
    {"help", ARGTYPE_EMPTY,"help",actionArg_help},
    {"disv", ARGTYPE_EMPTY, "display vtable", actionArg_printVtable},
    {"dish", ARGTYPE_EMPTY, "display list", actionArg_printList},
    {"disb", ARGTYPE_EMPTY, "display bytecode", actionArg_printBytecode},
    {"test", ARGTYPE_EMPTY, "for qcliu testing the VM", actionArg_test}
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

void printAllarg()
{
    int i = 0;
    for(; allArgs[i].action; i++)
    {
        printf("  -%s\t\t%s\n",allArgs[i].name,allArgs[i].desc);
    }
}

void actionArg_help()
{
    printAllarg();
    exit(0);
}




int j=0;
void commandline_doarg(int argc,char** argv)
{
    filename = argv;
    file_length = 0;
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
            //printf("error:the arg must start with '-'\n");
            //exit(0);
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
                        printf("ARGTYPE_INT no operate!");
                        exit(0);
                        break;
                    }
                case ARGTYPE_STRING:
                    {
                        index++;
                        char* arg = argv[index++];
                        allArgs[i].action(arg);
                        break;
                    }
                default:
                    printf("%s\n","impossible");
                    exit(0);
            }
            break;
        }
        if(!allArgs[i].action)
        {
            printf("error:no arg match!!!\n");
            exit(0);
        }

    }
}
