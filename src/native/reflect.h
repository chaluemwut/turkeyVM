#ifndef REF_H
#define REF_H
#include "../main/vm.h"

#define C Class_t


int instanceOf(Object* obj, C class);

Object* getClassConstructors(Object* vmClass, int isPublic);

#undef C
#endif
