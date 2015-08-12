#ifndef JSTRING_H
#define JSTRING_H

#include "../main/turkey.h"
#include "../heapManager/alloc.h"

#define O Object_t


char* Jstring2Char(O string);

O char2Char(char* s);

O createJstring(char* s);

O String_getValue(O);

int Jstring_equals(O s1, O s2);

void printStringObject(O obj);

void dumpJstring(O);

void dumpChar(O);




#undef O
#endif
