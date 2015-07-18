#ifndef TEST_H
#define TEST_H
#include "../main/vm.h"

#define C Class_t


//void printList(LinkedList* head);
//void printVtable(LinkedList* head);

void printStack();

void printNativeStack();

void printObjectWrapper(Object* objref);
void printObject(Object* objref);

void printString0(Object* obj);

void printChar0(Object* obj);

void dumpClass(C class);

#undef C
#endif
