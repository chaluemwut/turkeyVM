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
#include <stdlib.h>
#include "../classloader/class.h"
#include "../heapManager/alloc.h"
#include "../main/turkey.h"
#include "../util/exception.h"
#include "native.h"
#include "../classloader/resolve.h"
#include "../lib/assert.h"

#define C Class_t
#define O Object_t

/**
 * @parm
 *      mbtype the method's description.
 *
 * @return args count.
 * @see getClassConstructors
 */
static int getArgsCount(char* mbtype)
{
    /*{{{*/
    int args_count = 0;
    char* ptr = mbtype;
    ptr++;

    // step1: culucating the args count.
    while (*ptr != ')')
    {
        if (*ptr == '[')
        {
            do
            {
                ptr++;
            }
            while (*ptr == '[');

            if (*ptr == 'L')
            {
                do
                {
                    ptr++;
                }
                while (*ptr != ';');

            }

            ptr++;
            args_count++;
        }
        else if (*ptr == 'L')
        {
            do
            {
                ptr++;
            }
            while (*ptr != ';');
            ptr++;
            args_count++;

        }
        else
        {
            ptr++;
            args_count++;
        }
    }

    return args_count;
    /*}}}*/

}

/**
 *  @parm
 *      desc the mb->type
 *      argcount the arg's number
 * @return a array of char*, contains arg's name.
 * @see getClassConstructors()
 */
static char** getArgsName(char* desc, int argcount)
{
    /*{{{*/
    char** names = (char**)sysMalloc(sizeof(int)*(argcount+1));
    names[argcount] = 0;
    int index = 0;
    char* ptr = desc;

    ptr++;
    while (*ptr != ')')
    {
        if (index == argcount)
        {
            DEBUG("%s", "index, error");
            throwException("index error");
        }
        switch (*ptr)
        {
        case 'B':
        {
            char* name = "java/lang/Byte";
            names[index++] = name;
            ptr++;
            break;
        }
        case 'C':
        {
            char* name = "java/lang/Char";
            names[index++] = name;
            ptr++;
            break;
        }
        case 'D':
        {
            char* name = "java/lang/Double";
            names[index++] = name;
            ptr++;
            break;
        }
        case 'F':
        {
            char* name = "java/lang/Float";
            names[index++] = name;
            ptr++;
            break;
        }
        case 'I':
        {
            char* name = "java/lang/Integer";
            names[index++] = name;
            ptr++;
            break;
        }
        case 'J':
        {
            char* name = "java/lang/Long";
            names[index++] = name;
            ptr++;
            break;
        }
        break;
        case 'S':
        {
            char* name = "java/lang/Short";
            names[index++] = name;
            ptr++;
            break;
        }
        case 'Z':
        {
            char* name = "java/lang/Boolean";
            names[index++] = name;
            ptr++;
            break;
        }
        case '[':
        {
            char* p = ptr;
            int size = 0;
            do
            {
                size++;
                ptr++;
            }
            while (*ptr == '[');

            if (*ptr == 'L')
            {
                do
                {
                    size++;
                    ptr++;
                }
                while (*ptr != ';');
            }

            char* name = (char*)sysMalloc(size);
            memcpy(name, p, size);
            name[size] = '\0';
            names[index++] = name;

            ptr++;

            break;
        }
        case 'L':
        {
            ptr++;
            char* p = ptr;
            int size = 0;

            do
            {
                size++;
                ptr++;
            }
            while (*ptr != ';');

            char* name = (char*)sysMalloc(size+1);
            memcpy(name, p, size);
            name[size] = '\0';

            names[index++] = name;

            ptr++;
            break;
        }
        default:
            throwException("type array");

        }
    }

    return names;
    /*}}}*/
}

/**
 * @parm
 *      vmClass the header of Class which in the method area.
 *      vmClass->binding is the obj which we make Constructors to.
 *
 * @return an Constructor[].
 * @see native.c getDeclaredConstructors()
 *
 */
O getClassConstructors(O vmClass, int isPublic)
{
    /*{{{*/
    C array_class = findArrayClass("[Ljava/lang/reflect/Constructor;");
    C reflect_class = loadClass("java/lang/reflect/Constructor");
    /**/
    O c = (O)vmClass->binding;

    ClassBlock_t *cb = CLASS_CB(c->binding);

    O array, *cons;
    MethodBlock_t *init_mb;
    int count = 0;
    int i, j;

    if(!array_class || !reflect_class)
        return NULL;

    //find java/lang/reflect/Constrctor 's <init>
    /*
     * in face, this is no need. We can init  manually
     */
    if(!(init_mb = findMethod(reflect_class, "<init>", "(Ljava/lang/Class;I)V")))
        return NULL;

    // setp1 sum the count of constrctor for objclass
    for(i = 0; i < cb->methods_count; i++)
    {
        MethodBlock_t *mb = &cb->methods[i];
        if((strcmp(mb->name, "<init>") == 0) && (!isPublic || (mb->access_flags & ACC_PUBLIC)))
            count++;
    }

    //create a new array of java/lang/reflect/Constructor
    if((array = (O)allocArray(array_class, count, 4, T_REFERENCE)) == NULL)
        return NULL;

    cons = (O*)INST_DATA(array);

    for(i = 0, j = 0; j < count; i++)
    {
        MethodBlock_t *mb = &cb->methods[i];

        if((strcmp(mb->name, "<init>") == 0))
        {
            O reflect_ob;

            if((reflect_ob = allocObject(reflect_class)))
            {
                /**
                 * @see java.lang.reflect.Constructor
                 */
                FieldBlock_t* paramType = findField(reflect_class, "parameterTypes", "[Ljava/lang/Class;");
                FieldBlock_t* clazz = findField(reflect_class, "clazz", "Ljava/lang/Class;");
                FieldBlock_t* slot = findField(reflect_class, "slot", "I");

                int argcount = getArgsCount(mb->type);
                C class = loadClass("[Ljava/lang/Class;");
                O classarray = (O)allocArray(class, argcount, sizeof(int), T_REFERENCE);
                char** argname = getArgsName(mb->type, argcount);

                int i = 0;
                for (; i < argcount; i++)
                {
                    O arg_x = getClass_name(argname[i]);
                    ARRAY_DATA(classarray, i, O) = arg_x;
                }

                /*NOTE: vmClass->binding is a Class Object */
                OBJECT_DATA(reflect_ob, clazz->offset-1, O) = (O)vmClass->binding;
                /**
                 * @see class.c defineClass()
                 * 2015/07/01
                 */
                OBJECT_DATA(reflect_ob, slot->offset-1, int) = mb->slot;
                OBJECT_DATA(reflect_ob, paramType->offset-1, O) = classarray;


                cons[j++] = reflect_ob;
            }
            else
                return NULL;
        }
    }
    return array;
    /*}}}*/
}


/* return an object's class name*/
char* getObjectClassName(O obj)
{
    C class = obj->class;
    ClassBlock_t* cb = CLASS_CB(class);
    return cb->this_classname;
}



/* invoke by: OPC_INSTANCEOF */
int instanceOf(O obj, C class)
{

    return 1;//TODO
    //obj must non-null.
    int result;
    char* t_name;
    ClassBlock_t* obj_cb;
    ClassBlock_t* cb;


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




#undef C
#undef O
