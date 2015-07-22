#ifndef REF_H
#define REF_H
#include "../main/vm.h"
#include "../heapManager/alloc.h"

#define C Class_t
#define O Object_t


int instanceOf(O obj, C class);

O getClassConstructors(O vmClass, int isPublic);

#undef C
#undef O
#endif
