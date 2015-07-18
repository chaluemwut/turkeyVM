#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "../heapManager/alloc.h"
#include "../main/vm.h"
#include "../lib/list.h"
#include "dll.h"
#include "../lib/poly.h"

#define P Poly_t

static int equals(P x, P y)
{
    char* s = (char*)x;
    DllEntry* d = (DllEntry*)y;
    if (0 == strcmp(s, d->name))
      return 1;
    else
      return 0;
}

DllEntry* findDllInTable(char* dllname)
{
    DllEntry* dll = (DllEntry*)List_contains(DList, dllname, equals);

    return dll;
}

char* getDllPath() {
    return getenv("LD_LIBRARY_PATH");
}

int resolveDll(char* dllname) {
    DllEntry* dll;
    dll = findDllInTable(dllname);
    if (dll != NULL)
      return 1;

    void* handle = dlopen(dllname, RTLD_LAZY);
    if (!handle)
      return 0;

    dll = (DllEntry*)sysMalloc(sizeof(DllEntry));
    dll->name = dllname;
    dll->handle = handle;

    List_addLast(DList, dll);

    return 1;

}

char* getDllName(char* path, char* name) {
    char* buf = (char*)sysMalloc(strlen(path) + strlen(name) + 8);
    sprintf(buf, "%s/lib%s.so", path, name);

    return buf;
}

#undef P
