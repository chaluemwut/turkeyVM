/*------------------------------------------------------------------*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  resolve.h                              */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2015/01/04                             */
/*  DESCRIPTION           :  head for resolve.c                     */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 *
 */

#ifndef RESOLVE_H
#define RESOLVE_H

#include "../main/turkey.h"
#include "class.h"

#define C Class_t

C resolveClass(C class, u2 index);

MethodBlock* findMethod(C class, char* name, char* type);

MethodBlock* findMethodinCurrent(C class, char* name, char* type);

//int findField(C class, char* name, char* type);

//int findFieldinCurrent(C class, char* name, char* type);
FieldBlock* resolveField(C class, u2 index);

FieldBlock* findField(C class, char* name, char* type);

MethodBlock* resolveMethod(C class, u2 index);

MethodBlock* resolveInterfaceMethod(C class, u2 index);

#undef C
#endif
