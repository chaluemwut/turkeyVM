#ifndef TEST_H
#define TEST_H
#include <stdio.h>
#include "../main/turkey.h"
#include "../heapManager/alloc.h"
#include "../interp/stackmanager.h"

#define C Class_t
#define O Object_t
#define JF JFrame_t

void printStack(JF);

void printStackLog(FILE* fd, JF);

void printNativeStack();

void printObjectWrapper(O objref);
void printObject(O objref);

void printString0(O obj);

void printChar0(O obj);

#undef C
#undef O
#undef JF

#endif
