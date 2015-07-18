#ifndef ALLOC_H
#define ALLOC_H

#include "../main/vm.h"

#define C Class_t

/*Alloc*/

extern void* sysMalloc(int n);

extern Object* allocObject(C class);

extern Object* allocTypeArray(int type, int size, char* element_name);

extern Object* allocArray(C class, int size, int el_size, int atype);

#undef C
#endif
