#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include "../interp/stackmanager.h"
#include "../util/testvm.h"
#include "../control/control.h"
#include "../lib/list.h"
#include "../lib/poly.h"
#include "../lib/string.h"
#include "../lib/assert.h"
#include "../lib/error.h"

#define L List_t
#define P Poly_t

static L logList = NULL;


FILE* fd = NULL;
static FILE* temp = NULL;
static JFrame_t f;


void Log_add(char* s)
{
    if (!logList)
      logList = List_new();

    List_addFirst(logList, s);
}


int Log_contains(char* s)
{
    if (!logList)
      logList = List_new();

    int exist = List_contains(logList,
                s,
                (Poly_tyEquals)String_equals);

    return exist;
}

FILE* Log_open(char* s)
{
    if (fd != NULL)
    {
        temp = fd;
        fd = NULL;
        Control_setLogFile(fd);
        return 0;
    }


    if (!Log_contains(s))
    {
        fd = 0;
        Control_setLogFile(fd);
        return 0;
    }

    char* ss = String_concat("log_", "s", NULL);
    fd = fopen(ss, "a");
    temp = fd;
    f = getCurrentFrame();
    Assert_ASSERT(fd);


    Control_setLogFile(fd);

    return fd;
}


FILE* Log_recover()
{
    if (f != getCurrentFrame())
      return 0;

    Assert_ASSERT(temp);
    fd = temp;

    Control_setLogFile(fd);
    return fd;
}





#undef L
#undef P
