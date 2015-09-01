#ifndef RESOLVE_H
#define RESOLVE_H

#include "../main/turkey.h"
#include "class.h"

#define C Class_t

C resolveClass(C class, u2 index);

MethodBlock_t *findMethod(C class, char *name, char *type);

MethodBlock_t *findMethodinCurrent(C class, char *name, char *type);

MethodBlock_t *quickSearch(C class, char *name, char *type);

FieldBlock_t *resolveField(C class, u2 index);

FieldBlock_t *findField(C class, char *name, char *type);

MethodBlock_t *resolveMethod(C class, u2 index,
                             MethodBlock_t * (*)(C, char *, char *));

MethodBlock_t *resolveInterfaceMethod(C class, u2 index);

u4 resolveConstant(C class, int cp_index);

#undef C
#endif
