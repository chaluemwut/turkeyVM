#ifndef H_NATIVE
#define H_NATIVE
#include "../heapManager/alloc.h"

#define O Object_t

extern void getClass();

extern O getClass_name(char *classname);

extern void *findNativeInvoker(char *name, char *desc);

#undef O

#endif
