#ifndef EXE_H
#define EXE_H
#include "../main/vm.h"
#include "../heapManager/alloc.h"

#define C Class_t
#define O Object_t

/*execute*/
extern void executeMethod(MethodBlock* mb, va_list jargs);

extern void executeStaticMain(MethodBlock* mb);

extern void executeMethodArgs(C class, MethodBlock* mb,...);

extern void invoke(MethodBlock* mb, O args, O this);

extern void popFrame();

#undef C
#undef O

#endif