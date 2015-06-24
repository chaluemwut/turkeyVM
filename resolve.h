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
#include "vm.h"
Class* resolveClass(Class* class, u2 index);

MethodBlock* findMethod(Class* class, char* name, char* type);

MethodBlock* findMethodinCurrent(Class* class, char* name, char* type);

//int findField(Class* class, char* name, char* type);

//int findFieldinCurrent(Class* class, char* name, char* type);
FieldBlock* resolveField(Class* class, u2 index);

FieldBlock* findField(Class* class, char* name, char* type);

MethodBlock* resolveMethod(Class* class, u2 index);

MethodBlock* resolveInterfaceMethod(Class* class, u2 index);
