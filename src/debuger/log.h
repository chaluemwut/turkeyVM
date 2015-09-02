#ifndef LOG_H
#define LOG_H
#include <stdio.h>

#define Log_SingleMethod(s, f, x, r)        \
    do{                                         \
        Log_open(s);                            \
        r =f x;                                 \
        Log_recover();                          \
    }while(0)

extern void Log_add(char *s);

extern int Log_contains(char *s);

extern FILE *Log_open(char *s);

extern FILE *Log_recover();

#endif
