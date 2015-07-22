#ifndef TEST_H
#define TEST_H
#include "../main/vm.h"
#include "../heapManager/alloc.h"

#define C Class_t
#define O Object_t


//void printList(LinkedList* head);
//void printVtable(LinkedList* head);

void printStack();

void printNativeStack();

void printObjectWrapper(O objref);
void printObject(O objref);

void printString0(O obj);

void printChar0(O obj);

void dumpClass(C class);

#undef C
#undef O
#endif
