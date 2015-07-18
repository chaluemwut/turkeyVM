#ifndef DLL_H
#define DLL_H

#include "../main/vm.h"



typedef struct dllEntry
{
    char* name;
    char* handle;
}DllEntry;


/*dll*/
extern DllEntry* findDllInTable(char* dllname);
extern char* getDllName(char* path, char* name);
extern int resolveDll(char* dllname);
extern char* getDllPath();

#endif
