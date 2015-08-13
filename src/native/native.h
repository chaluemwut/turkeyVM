#ifndef H_NATIVE
#define H_NATIVE
#include "../heapManager/alloc.h"

#define O Object_t


typedef struct
{
    char* method_name;
    char* desc;
    void (*action)();
}Binding;


extern Binding nativeMethods[];

extern void getClass();
extern O getClass_name(char* classname);

#undef O

#endif
