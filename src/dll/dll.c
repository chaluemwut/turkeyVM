#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "dll.h"
#include "../heapManager/alloc.h"
#include "../main/turkey.h"
#include "../lib/list.h"
#include "../lib/poly.h"
#include "../lib/hash.h"
#include "../lib/string.h"
#include "../lib/error.h"

#define P Poly_t
#define D Dll_t

static Hash_t DMap;

static int equals(P x, P y)
{
    char* s = (char*)x;
    D d = (D)y;
    if (0 == strcmp(s, d->name))
        return 1;
    else
        return 0;
}

D findDllInTable(char* dllname)
{
    D dll = (D)Hash_get(DMap, dllname);

    return dll;
}

char* getDllPath()
{
    return getenv("LD_LIBRARY_PATH");
}

int resolveDll(char* dllname)
{
    D dll;
    dll = findDllInTable(dllname);
    if (dll != NULL)
        return 1;

    void* handle = dlopen(dllname, RTLD_LAZY);
    if (!handle)
        return 0;

    dll = (D)sysMalloc(sizeof(struct D));
    dll->name = dllname;
    dll->handle = handle;

    Hash_put(DMap, dll->name, dll);

    return 1;

}

char* getDllName(char* path, char* name)
{
    char* buf = (char*)sysMalloc(strlen(path) + strlen(name) + 8);
    sprintf(buf, "%s/lib%s.so", path, name);

    return buf;
}

static void keyDup(P x, P y)
{
    ERROR("dup key");
}

void initDllHash()
{
    DMap = Hash_new((long(*)(P))String_hashCode
                    ,(Poly_tyEquals)String_equals
                    ,keyDup);
}

#undef P
#undef D
