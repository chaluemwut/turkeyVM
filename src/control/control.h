#ifndef CONTROL_H
#define CONTROL_H

extern int dis_list;
extern int dis_vtable;
extern int dis_bytecode;
extern int dis_testinfo;
extern int assert_stack;

extern char* op_code[];

typedef enum {
    VERBOSE_SILENT,
    VERBOSE_PASS,
    VERBOSE_SUBPASS,
    VERBOSE_DETAIL
}Verbose_t;

int Control_Verb_order(Verbose_t v1, Verbose_t v2);

extern Verbose_t Control_verbose;

#endif
