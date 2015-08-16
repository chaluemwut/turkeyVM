#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "testvm.h"
#include "../main/turkey.h"
#include "../classloader/class.h"
#include "../lib/hash.h"
#include "../interp/stackmanager.h"

extern JFrame_t current_frame;

void throwException(char* exception) {
    printf("\n\e[31m\e[1mException:\e[0m %s\n", exception);

    classHashStatus();

    exit(0);
}

void Exception(char* fmt, ...)
{
    fprintf(stderr, "\e[31m\e[1mException:\e[0m");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    fflush(stderr);

    exit(1);
}
