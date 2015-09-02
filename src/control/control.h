#ifndef CONTROL_H
#define CONTROL_H
#include <stdio.h>

extern int dis_testinfo;
extern int assert_stack;

typedef enum {
    VERBOSE_SILENT,
    VERBOSE_PASS,
    VERBOSE_SUBPASS,
    VERBOSE_DETAIL
} Verbose_t;

extern int Control_Verb_order(Verbose_t v1, Verbose_t v2);

extern Verbose_t Control_verbose;

extern void Control_setLogFile(FILE * fd);

extern int Control_opcode;

extern int Control_resolve;

#endif
