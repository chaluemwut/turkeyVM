#ifndef DUMP_H
#define DUMP_H
#include "../classloader/class.h"
#include "../heapManager/alloc.h"

#define C Class_t
#define O Object_t

extern void dumpClass(FILE *, char *, C);
extern void dumpObject(O obj);

#undef C
#undef O

#endif
