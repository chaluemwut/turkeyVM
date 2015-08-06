#ifndef TEST_H
#define TEST_H
#include "../main/turkey.h"
#include "../heapManager/alloc.h"
#include "../interp/stackmanager.h"

#define C Class_t
#define O Object_t
#define JF JFrame_t

void printStack(JF);

void printNativeStack();

void printObjectWrapper(O objref);
void printObject(O objref);

void printString0(O obj);

void printChar0(O obj);

void dumpClass(C class);

#undef C
#undef O
#undef JF

#endif
