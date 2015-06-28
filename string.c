/*------------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  string.c                               */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2015/03/08                             */
/*  DESCRIPTION           :  for the String.class                   */
/*------------------------------------------------------------------*/

/*
 * 
 *
 *//*}}}*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "string.h"
#include "vm.h"
#include "resolve.h"
#include "exception.h"

#include "testUTF8.h"

extern Class* java_lang_String;
static int count_offset;
static int value_offset;
static int offset_offset;
static int inited;

static void initString() {
    if (java_lang_String == NULL)
      java_lang_String = loadClass("java/lang/String");
    FieldBlock* count;
    FieldBlock* value;
    FieldBlock* offset;

    count = (FieldBlock*)findField(java_lang_String, "count", "I");
    value = (FieldBlock*)findField(java_lang_String, "value", "[C");
    offset =(FieldBlock*)findField(java_lang_String, "offset", "I");

    if ((count == NULL)||(value == NULL)||(offset == NULL))
      throwException("initString error");

    count_offset = count->offset;
    value_offset = value->offset;
    offset_offset = offset->offset;

    inited = TRUE;

}

/*Object* createString0(char* utf8)
  {
  int len = strlen(utf8);
  Object* array;
  Object* obj;

  if((array = (Object*)allocTypeArray(T_CHAR, len))==NULL
  || (obj = allocObject(java_lang_String)) == NULL)
  throwException("@ createString");

  int i;
  for (i = 0; i < len; i++)
  {
 *((char*)(unsigned short*)array->data+i) = utf8[i];
 }

 }*/

/**
 * 
 * @return new char
 */
char* String2Char(Object* string) {
    Object* array = (Object*)(INST_DATA(string)[value_offset-1]);
    int len = INST_DATA(string)[count_offset-1];
    int offset = INST_DATA(string)[offset_offset-1];
    char* cstr = (char*)sysMalloc(len+1), *spntr;
    short* str = ((short*)INST_DATA(array))+offset;

    for (spntr = cstr; len > 0; len--)
      *spntr++ = *str++;

    *spntr = '\0';

    return cstr;
}
/*
 * Create a new array which type is [C.
 * In interp.c's OPC_LDC.
 * @qcliu 2015/03/08
 * invoked by:creatString();
 */
Object* char2Char(char* s) {
    Class* class;
    int length;
    Object* obj;
    int i;

    length = strlen(s);

    obj = (Object*)allocTypeArray(T_CHAR, length, NULL);

    for (i = 0; i < length; i++) {
        *((char*)(unsigned short*)obj->data+i) = s[i];
    }

    return obj;

}

/*Create a new String
 *invoked by:OPC_LDC
 */
Object* createString(char* s) {
    if (java_lang_String == NULL)
      java_lang_String = loadClass("java/lang/String");

    ClassBlock* cb = CLASS_CB(java_lang_String);

    if (!inited)
      initString();

    Object *char_obj, *string_obj;
    FieldBlock* fb;
    int length, offset;

    length = strlen(s);
    char_obj = char2Char(s);

    short* data = (short*)char_obj->data;
    unsigned char* ss = (unsigned char*)s;
    convertUtf8(ss, data);

    string_obj = allocObject(java_lang_String);
    string_obj->isArray = 2;
    OBJECT_DATA(string_obj, value_offset-1, Object*) = char_obj;
    OBJECT_DATA(string_obj, count_offset-1, int) = length;
    //*(((Object**)string_obj->data)+offset-1) = char_obj;

    string_obj->cb = cb;
    string_obj->el_size = sizeof(int);

    //printString0(string_obj);

    return string_obj;


}

/* Given a String object, and print*/
void printStringObject(Object* obj) {

    int offset;
    Object* char_obj;
    FieldBlock* fb = (FieldBlock*)findField(java_lang_String, "value", "[C");
    offset = fb->offset;
    char_obj = *(((Object**)obj->data)+offset-1);
    printf("String:%s\n", (char*)char_obj->data);

}
