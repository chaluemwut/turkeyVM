#ifndef CLASS_H
#define CLASS_H
#include "../main/turkey.h"

#define C Class_t

#define CLASSNAME(c) (((ClassBlock*)(c+1))->this_classname)



extern void initClass(C class);


extern C loadClass(char* classname);

extern C findArrayClass(char* classname);

extern C findPrimitiveClass(char primtype);

extern int parseArgs(char* type);

extern C findClass(char* classname);

extern char* getClassPath();

extern void parseClassPath(char*);

extern void parseFilename(char*);


#undef C

#endif
