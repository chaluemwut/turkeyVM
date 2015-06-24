/*------------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  reflect.c                              */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2015/03/24                             */
/*  DESCRIPTION           :  for the javaVM                         */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 *
 *//*}}}*/

#include <stdio.h>
#include <string.h>
#include "vm.h"
#include "exception.h"
#include "native.h"


/* return an object's class name*/
char* getObjectClassName(Object* obj)
{
    Class* class = obj->class;
    ClassBlock* cb = CLASS_CB(class);
    return cb->this_classname;
}


/* invoke by: OPC_INSTANCEOF */
int instanceOf(Object* obj, Class* class)
{

    return 1;
    //obj must non-null.
    int result;
    char* t_name;
    ClassBlock* obj_cb;
    ClassBlock* cb;


    cb = CLASS_CB(class);
    obj_cb = CLASS_CB(obj->class);
    t_name = cb->this_classname;

    if (IS_INTERFACE(cb))
    {
        throwException("instanceof interface");
    }
    else if (IS_ARRAY(cb))
    {
        throwException("instanceof array");
    }
    else
    {

        if (strcmp(obj_cb->this_classname, t_name) == 0)
        {
            result = 1;
            return result;
        }
        else
          result = 0;

        while (obj_cb->super)
        {
            obj_cb = CLASS_CB(obj_cb->super);
            if (strcmp(obj_cb->this_classname, t_name) == 0)
            {
                result = 1;
                break;
            }
        }
    }
    return result;



}




