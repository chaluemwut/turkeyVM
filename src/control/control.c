#include "control.h"
#define FALSE 0
#define TRUE 1

/*
 * To control weather print vtable and ClassList.
 * Be used in vm.c.
 * @qcliu 2015/01/27
 */
int assert_stack = TRUE;
int dis_testinfo = FALSE;

/*{{{ Verbose*/
Verbose_t Control_verbose = VERBOSE_SILENT;
Verbose_t Control_verboseDefault = VERBOSE_SILENT;

int Control_Verb_order(Verbose_t v1, Verbose_t v2)
{
    return v1 <= v2;
}

/*}}}*/

/*{{{ Debuge*/

FILE *Log_file = NULL;

void Control_setLogFile(FILE * fd)
{
    Log_file = fd;
}

/*}}}*/

/*{{{opcode*/
int Control_opcode = FALSE;
/*}}}*/

int Control_resolve = TRUE;
