#ifndef STRING_H
#define STRING_H
#include "../main/vm.h"


char* String2Char(Object* string);
Object* char2Char(char* s);
Object* createString(char* s);
void printStringObject(Object* obj);




#endif
