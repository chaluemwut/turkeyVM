#ifndef EXE_H
#define EXE_H
#include "../main/vm.h"

#define C Class_t

/*execute*/
extern void executeMethod(MethodBlock* mb, va_list jargs);

extern void executeStaticMain(MethodBlock* mb);

extern void executeMethodArgs(C class, MethodBlock* mb,...);

extern void invoke(MethodBlock* mb, Object* args, Object* this);

extern void popFrame();

#undef C

#endif
