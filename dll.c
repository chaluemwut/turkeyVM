#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "vm.h"
#include "linkedlist.h"

extern LinkedList* DLL;

char* getDllPath()
{
    return getenv("LD_LIBRARY_PATH");
}

int resolveDll(char* dllname)
{
    DllEntry* dll;
    dll = findDllInTable(DLL, dllname);
    if (dll != NULL)
      return 1;

    void* handle = dlopen(dllname, RTLD_LAZY);
    if (!handle)
      return 0;

    dll = (DllEntry*)sysMalloc(sizeof(DllEntry));
    dll->name = dllname;
    dll->handle = handle;

    addLast(DLL, dll);

    return 1;

}

char* getDllName(char* path, char* name)
{
    char* buf = (char*)sysMalloc(strlen(path) + strlen(name) + 8);
    sprintf(buf, "%s/lib%s.so", path, name);

    return buf;
}
