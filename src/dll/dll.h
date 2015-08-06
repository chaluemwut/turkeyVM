#ifndef DLL_H
#define DLL_H

#include "../main/turkey.h"

#define D Dll_t

typedef struct D *D;

struct D
{
    char* name;
    char* handle;
};


/*dll*/
extern D findDllInTable(char* dllname);
extern char* getDllName(char* path, char* name);
extern int resolveDll(char* dllname);
extern char* getDllPath();

#undef D

#endif
