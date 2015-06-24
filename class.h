#ifndef CLASS_H
#define CLASS_H
#include "vm.h" 

void initClass(Class* class);

Class* loadClass_not_init(char* classname);

Class* loadClass(char* classname);

Class* findArrayClass(char* classname);

Class* findPrimitiveClass(char primtype);
 
#endif
