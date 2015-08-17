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
#include "jstring.h"
#include "../classloader/class.h"
#include "../heapManager/alloc.h"
#include "../main/turkey.h"
#include "../classloader/resolve.h"
#include "exception.h"
#include "../lib/assert.h"

#include "testUTF8.h"
#define C Class_t
#define O Object_t

static C java_lang_String;
static int count_offset;
static int value_offset;
static int offset_offset;
static int inited;

void initString()
{
    if (java_lang_String == NULL)
        java_lang_String = loadClass("java/lang/String");
    FieldBlock_t* count;
    FieldBlock_t* value;
    FieldBlock_t* offset;

    count = (FieldBlock_t*)findField(java_lang_String, "count", "I");
    value = (FieldBlock_t*)findField(java_lang_String, "value", "[C");
    offset =(FieldBlock_t*)findField(java_lang_String, "offset", "I");

    if ((count == NULL)||(value == NULL)||(offset == NULL))
        throwException("initString error");

    count_offset = count->offset;
    value_offset = value->offset;
    offset_offset = offset->offset;

    inited = TRUE;

}

O String_getValue(O obj)
{
    O array;
    array = OBJECT_DATA(obj, value_offset-1, O);
    return array;
}


/**
 *
 * @return new char
 */
char* Jstring2Char(O string)
{
    O array = (O)(INST_DATA(string)[value_offset-1]);
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
O char2Char(char* s)
{
    C class;
    int length;
    O obj;
    int i;

    length = strlen(s);

    obj = (O)allocTypeArray(T_CHAR, length, NULL);

    for (i = 0; i < length; i++)
    {
        *((char*)(unsigned short*)obj->data+i) = s[i];
    }

    return obj;

}

/*Create a new String
 *invoked by:OPC_LDC
 */
O createJstring(char* s)
{
    if (java_lang_String == NULL)
        java_lang_String = loadClass("java/lang/String");

    ClassBlock_t* cb = CLASS_CB(java_lang_String);

    if (!inited)
        initString();

    O char_obj, string_obj;
    FieldBlock_t* fb;
    int length, offset;

    length = strlen(s);
    char_obj = char2Char(s);

    short* data = (short*)char_obj->data;
    unsigned char* ss = (unsigned char*)s;
    convertUtf8(ss, data);

    string_obj = allocObject(java_lang_String);
    string_obj->type = OBJECT_STRING;
    OBJECT_DATA(string_obj, value_offset-1, O) = char_obj;
    OBJECT_DATA(string_obj, count_offset-1, int) = length;
    //*(((Object**)string_obj->data)+offset-1) = char_obj;

    string_obj->cb = cb;
    string_obj->el_size = sizeof(int);


    return string_obj;


}

/* Given a String object, and print*/
//FIXME
void printStringObject(O obj)
{

    int offset;
    O char_obj;
    FieldBlock_t* fb = (FieldBlock_t*)findField(java_lang_String, "value", "[C");
    offset = fb->offset;
    char_obj = *(((O*)obj->data)+offset-1);
    printf("String:%s\n", (char*)char_obj->data);

}

int Jstring_equals(O s1, O s2)
{
    Assert_ASSERT(s1&&s1->type==OBJECT_STRING);
    Assert_ASSERT(s2&&s2->type==OBJECT_STRING);
    char* c1 = Jstring2Char(s1);
    char* c2 = Jstring2Char(s2);

    if (0 == strcmp(c1, c2))
      return 1;
    else 
      return 0;


}


/** 
 * print char[].
 * @parm obj an Object_t, and type must be array.
 */
void dumpChar(O obj)
{

    Assert_ASSERT(obj);
    Assert_ASSERT(obj->type == OBJECT_ARRAY);
    short* p = (short*)obj->data;
    int i;
    for (i = 0; i < obj->length; i++)
    {
        char c = (char)(*p);
        printf("%c", c);
        p++;
    }
    printf("\n");
}


/**
 *
 */
void dumpJstring(O obj)
{
    Assert_ASSERT(obj);
    Assert_ASSERT(value_offset);

    O c = OBJECT_DATA(obj, value_offset-1, O);

    dumpChar(c);
}

#undef C
#undef O
