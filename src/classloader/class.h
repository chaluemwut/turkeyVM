#ifndef CLASS_H
#define CLASS_H
#include "../main/vm.h"

#define C Class_t

extern void initClass(C class);

extern C loadClass_not_init(char* classname);

extern C loadClass(char* classname);

extern C findArrayClass(char* classname);

extern C findPrimitiveClass(char primtype);

extern int parseArgs(char* type);

extern C findClass(char* classname);

#undef C

#endif
