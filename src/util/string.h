#ifndef STRING_H
#define STRING_H

#include "../main/turkey.h"
#include "../heapManager/alloc.h"

#define O Object_t


char* String2Char(O string);
O char2Char(char* s);
O createString(char* s);
O String_getValue(O);
void printStringObject(O obj);




#undef O
#endif
