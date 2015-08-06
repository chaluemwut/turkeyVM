/*^_^*--------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  class.c                                */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2014/12/31                             */
/*  DESCRIPTION           :  for the javaVM                         */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 *
 *2015/01/06
 *Add the prepareClass() and linkClass().
 *2015/01/16
 *Add prepare field in prepareClass(),
 *2015/01/20
 *Add loadArrayClass();
 *//*}}}*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../interp/execute.h"
#include "../heapManager/alloc.h"
#include "../main/turkey.h"
#include "resolve.h"
#include "../lib/list.h"
#include "../lib/poly.h"
#include "../util/exception.h"
#include "class.h"
#include "../native/native.h"
#include "../control/control.h"
#include "../lib/error.h"
#include "../lib/string.h"

#define MAX_PRIMITIVE 9
#define C Class_t
#define O Object_t
#define P Poly_t

static const int ClassHeaderSize = sizeof(struct C);

/* This ia an talbe for primitiveClass, like
 * int, long, double, float, boolean, short, char...
 * When load primitiveClass, search this table.
 * @qcliu 2015/03/22
 */
C primClass[MAX_PRIMITIVE];

static char** CLASSPATH = NULL;
static int MAX_PATH_LEN = 0;
static char* PREFIX = NULL;

//method
static C loadArrayClass(char* classname);
C findArrayClass(char* classname);

//extern
extern int initable;

static u2* for_test;


//static char* test;

C findClass(char* classname)
{
    C c = (C)Hash_get(CMap, classname);
    return c;
}

void parseFilename(char* s)
{
    /*{{{*/
    char* p = s;
    int i = 0;
    int k = 0;
    while (*p)
    {
        if (*p == '/')
          k = i;

        p++;
        i++;
    }

    if (k == 0)
      return;

    PREFIX = (char*)malloc(k+2);
    strncpy(PREFIX, s, k+1);
    PREFIX[k+1] = '\0';
    /*}}}*/
}

void parseClassPath(char* c)
{
    /*{{{*/
    CLASSPATH = String_split(c, ":");
    int i = 0;
    while (CLASSPATH[i])
    {
        if (strlen(CLASSPATH[i])>MAX_PATH_LEN)
          MAX_PATH_LEN = strlen(CLASSPATH[i]);
        i++;
    }
    /*}}}*/
}


/*
 * Calculate the count of the method. The args is the method->type.
 * note: the Double and Float need two slot in the stack.
 * @qcliu 2015/01/28
 */
int parseArgs(char* args)
{
    char* ptr = args;/*{{{*/
    int count = 0;

    /*
     * (ParameterDesciptor*) ReturnDescriptor
     * The args must start with '(' and end with ')'
     */
    while (*ptr != ')')
    {
        if ((*ptr == 'I') || (*ptr == 'C') || (*ptr == 'B') ||
                (*ptr == 'F') || (*ptr == 'S') || (*ptr == 'Z'))
        {
            count++;
            ptr++;
        }
        else if ((*ptr == 'J') || (*ptr == 'D'))
        {
            count += 2;
            ptr++;
        }
        else if (*ptr == 'L')
        {
            //NOTE: find the first 'L', then omit 'L' until find ';'.
            count++;
            /* referentce type must end by ';'*/
            while (*ptr != ';')
                ptr++;
            ptr++;
        }
        else if (*ptr == '[')
        {
            count++;

            do
            {
                ptr++;
            }
            while (* ptr == '[');

            if (*ptr == 'L')
            {
                do
                {
                    ptr++;
                }
                while (*ptr != ';');
            }

            ptr++;

        }
        //TODO need??
        else
        {
            ptr++;
        }
    }

    return count;/*}}}*/
}

/*
 * Established the data strcture for the classfile in mem.
 *
 * invoke by: loadSystemClass()
 * @qcliu 2015/01/27
 */
static C defineClass(char* classname, char* data, int file_len)
{
    /*{{{*/
    unsigned char* ptr = (unsigned char*)data;
    u4 magic;
    u2 major_v;
    u2 minor_v;
    u2 methodsCount;
    u2 cp_count;
    u2 interfaces_count;
    u2 fieldsCount;
    u2 attrCount;
    int i;
    ClassBlock* classblock;
    C class;//this is the return class
    ConstantPool* constantPool;
    u2* interfaces_idx;
    FieldBlock* fields;
    MethodBlock* methods;

    READ_U4(magic, ptr);
    if (magic != 0xcafebabe)
        throwException("NoClassDefFound");
    READ_U2(minor_v, ptr);
    READ_U2(major_v , ptr);
    class = sysMalloc(ClassHeaderSize+sizeof(ClassBlock));
    classblock = CLASS_CB(class);

    //init
    classblock->magic = magic;
    classblock->minor_version = minor_v;
    classblock->major_version = major_v;
    classblock->flags = 0;
    classblock->methods_table = NULL;
    classblock->element = NULL;
    classblock->dim = 0;
    classblock->super = NULL;
    classblock->this_classname = classname;//mast read from constant pool
    classblock->type_flags = 0;
    classblock->interface_count = 0;

    /*-----------------constant_pool---------------------*/
    READ_U2(cp_count, ptr);/*{{{*/
    //printf("constant_pool_count:%d\n", cp_count);
    classblock->constant_pool_count = cp_count;

    constantPool = &classblock->constant_pool;
    constantPool->type = (volatile u1*)sysMalloc(cp_count);
    constantPool->info = (ConstantPoolEntry*)
                          sysMalloc(cp_count * sizeof(ConstantPoolEntry));

    for (i=1; i<cp_count; i++)
    {
        u1 tag;
        READ_U1(tag, ptr);
        CP_TYPE(constantPool, i) = tag;

        switch (tag)
        {
        case CONSTANT_Class:
        case CONSTANT_String:
        case CONSTANT_MethodType:
            READ_INDEX(CP_INFO(constantPool, i), ptr);
            break;
        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_NameAndType:
        case CONSTANT_InterfaceMethodref:
        case CONSTANT_InvokeDynamic:
        {
            u2 idx1, idx2;
            READ_INDEX(idx1, ptr);
            READ_INDEX(idx2, ptr);
            CP_INFO(constantPool, i) = idx2<<16|idx1;
            break;
        }
        case CONSTANT_Integer:
            READ_U4(CP_INFO(constantPool, i),  ptr);
            break;
        case CONSTANT_Float:
            READ_U4(CP_INFO(constantPool, i),  ptr);
            break;

            /*NOTE: Long and Double use two step to obtain.
             *
             *@qcliu 2015/03/15
             */
        case CONSTANT_Long:
            READ_U4(CP_INFO(constantPool, i+1), ptr);
            READ_U4(CP_INFO(constantPool, i), ptr);
            i++;
            break;
        case CONSTANT_Double:
            READ_U4(CP_INFO(constantPool, i+1), ptr);
            READ_U4(CP_INFO(constantPool, i), ptr);
            i++;
            break;
        case CONSTANT_Utf8:
        {
            int length;
            unsigned char* utf8;

            READ_U2(length, ptr);
            utf8=(unsigned char*)sysMalloc(length + 1);
            memcpy(utf8, ptr, length);
            utf8[length]='\0';
            ptr += length;

            CP_INFO(constantPool,i) = (u4)utf8;
            break;
        }
        case CONSTANT_MethodHandle:
        {
            u2 ref_kind;
            u2 ref_idx;
            READ_U1(ref_kind, ptr);
            READ_U2(ref_idx, ptr);
            CP_INFO(constantPool, i) = ref_idx<<16 || ref_kind;
            break;
        }
        default:
            printf("%d\n", i);
            throwException("InvalidConstantPoolIndex");
        }


    }
    /*}}}*/

    /*---------------------------access_flag,this_class,super_class------------*/
    READ_U2(classblock->access_flags, ptr);/*{{{*/
    u2 this_classidx, super_classidx, super_name_idx;
    READ_U2(this_classidx, ptr);
    READ_U2(super_classidx, ptr);
    //printf("this_classidx:%d\n", this_classidx);
    //printf("super_classidx:%d\n", super_classidx);
    this_classidx = CP_INFO(constantPool, this_classidx);
    super_name_idx = CP_INFO(constantPool, super_classidx);
    classblock->this_classname = CP_UTF8(constantPool, this_classidx);
    classblock->super_classname = CP_UTF8(constantPool, super_name_idx);
    //resovle/*}}}*/

    /*----------------------------interface------------------------------------*/
    u2 idx;/*{{{*/
    READ_U2(interfaces_count, ptr);
    classblock->interface_count = interfaces_count;
    //printf("interface_count:%d\n", interfaces_count);
    //interfaces_idx=classblock->interfaces_idx;
    //interfaces_idx=(u2*)sysMalloc(interfaces_count * sizeof(u2));

    classblock->interfaces = (C*)sysMalloc(sizeof(struct Class_t) * interfaces_count);

    for (i=0; i<interfaces_count; i++)
    {
        READ_U2(idx, ptr);

        classblock->interfaces[i] = resolveClass(class,idx);
    }
    /*}}}*/

    /*--------------------------fields------------------------------------------*/
    READ_U2(classblock->fields_count, ptr);/*{{{*/
    //printf("fields_count:%d\n", classblock->fields_count);
    fields = sysMalloc(classblock->fields_count * sizeof(FieldBlock));
    classblock->fields = fields;

    for (i = 0; i<classblock->fields_count; i++)
    {
        u2 name_idx, type_idx;
        u2 attr_count;

        u2 attr_name_idx;
        u4 attr_length;

        READ_U2(fields[i].access_flags, ptr);
        READ_INDEX(name_idx, ptr);
        READ_INDEX(type_idx, ptr);
        fields[i].name = CP_UTF8(constantPool, name_idx);
        fields[i].type = CP_UTF8(constantPool, type_idx);
        //printf("field_name:%s\n", fields[i].name);
        //printf("field_type:%s\n", fields[i].type);
        //init the field
        fields[i].constant = 0;
        fields[i].static_value = 0;
        fields[i].offset = 0;

        READ_U2(attr_count, ptr);
        //printf("attr_count:%d\n", attr_count);

        while (attr_count != 0)
        {
            READ_U2(attr_name_idx, ptr);
            READ_U4(attr_length, ptr);
            //printf("attr_length:%d\n", attr_length);

            char* attr_name = CP_UTF8(constantPool, attr_name_idx);
            //printf("%s\n", attr_name);

            //this field is static final
            if (strcmp(attr_name, "ConstantValue") == 0 )
            {
                READ_U2(fields[i].constant, ptr);
                //printf("%d\n", fields[i].constant_idx);
            }
            else
            {
                ptr += attr_length;
            }
            attr_count--;
        }

    }
    /*}}}*/

    /*-------------------------------------Method----------------*/
    READ_U2(methodsCount, ptr);/*{{{*/
    //printf("methods_count:%d\n", methods_count);
    classblock->methods_count = methodsCount;
    methods = (MethodBlock*)sysMalloc(methodsCount * sizeof(MethodBlock));
    classblock->methods = methods;

    for (i = 0; i < methodsCount; i++)
    {
        u2 access_flags, name_idx, type_idx, attr_count;
        READ_U2(access_flags, ptr);
        READ_U2(name_idx, ptr);
        READ_U2(type_idx, ptr);
        READ_U2(attr_count, ptr);

        methods->access_flags = access_flags;

        methods->name = CP_UTF8(constantPool, name_idx);

        /*@TEST
         *
         * 2015/07/06
         * */
        //if (0 == strcmp(methods->name, "constructNative"))
        //DEBUG("constructNative");

        methods->type = CP_UTF8(constantPool, type_idx);
        methods->code_length = 0;
        methods->native_invoker = 0;
        methods->max_locals = 0;
        methods->max_stack = 0;


        /**
         * @see native.c
         * 2015/07/01
         */
        methods->slot = i;
        /*
         * When get the description of the method, need to calculate the
         * count of the args of the method.
         * @qcliu 2015/01/27
         */
        methods->args_count = parseArgs(methods->type);

        //========================================================================
        /*
         * Binding the native method to the mb->native_invoker
         * @qcliu 2015/03/06
         */
        if (methods->access_flags & ACC_NATIVE)
        {
            int j;
            for (j = 0; nativeMethods[j].action; j++)
            {
                if ((strcmp(nativeMethods[j].method_name, methods->name) == 0) &&
                        (strcmp(nativeMethods[j].desc, methods->type) == 0))
                {
                    methods->native_invoker = nativeMethods[j].action;
                    break;
                }
            }
        }
        //====================================================================

        methods->class = class;
        //ClassBlock* testcb = CLASS_CB(class);
        //printf("%s\n", testcb->this_classname);
        //printf("method_name:%s\n", methods->name);
        //printf("method_type:%s\n", methods->type);
        //method_attr
        for (; attr_count != 0; attr_count--)
        {
            u2 attr_name_idx;
            u4 attr_length;
            char* attr_name;
            READ_INDEX(attr_name_idx, ptr);
            READ_U4(attr_length, ptr);
            attr_name = CP_UTF8(constantPool, attr_name_idx);
            /*code*/
            if (strcmp(attr_name, "Code") == 0)
            {
                u4 code_length, j;
                u1* code;
                u2 exception_table_length;
                CodeException* exception;
                u2 code_attr_count;

                READ_U2(methods->max_stack, ptr) ;
                READ_U2(methods->max_locals, ptr);
                READ_U4(code_length, ptr);
                //printf("code_length:%d\n", code_length);

                code = (u1*)sysMalloc(code_length * sizeof(u1));
                for (j = 0; j < code_length; j++)
                {
                    READ_U1(code[j], ptr);
                }

                methods->code_length = code_length;
                methods->code = code;
                /*print*/
                for (j = 0; j < code_length; j++)
                {
                    //printf("%d ", methods->code[j]);
                }
                //printf("\n");

                READ_U2(exception_table_length, ptr);
                exception = (CodeException*)sysMalloc(exception_table_length
                                                      * sizeof(CodeException));
                for (j = 0; j < exception_table_length; j++)
                {
                    READ_U2(exception[j].start_pc, ptr);
                    READ_U2(exception[j].end_pc, ptr);
                    READ_U2(exception[j].handler_pc, ptr);
                    READ_U2(exception[j].catch_type, ptr);
                }
                methods->exception_table_length = exception_table_length;
                methods->code_exception = exception;
                /*code_arrt*/
                READ_U2(code_attr_count, ptr);
                for (j = 0; j < code_attr_count; j++)
                {
                    u2 attr_nameidx;
                    u4 len;
                    READ_INDEX(attr_nameidx, ptr);
                    char* temp = CP_UTF8(constantPool, attr_nameidx);
                    READ_U4(len, ptr);
                    ptr += len;
                }

            }

            else if (strcmp(attr_name, "Exceptions") == 0)
            {
                u2 exception_count , j;
                u2* exception_index_table;

                READ_U2(exception_count, ptr);
                exception_index_table = (u2*)sysMalloc(exception_count * sizeof(u2));
                for (j = 0; j < exception_count; j++)
                {
                    READ_U2(exception_index_table[j], ptr);
                }

                methods->number_of_exceptions = exception_count;
                methods->exception_idx_table = exception_index_table;
            }
            else
                /*
                 *@chuan
                 *omit other attributes
                 */
                ptr += attr_length;
        }
        methods++;
    }
    /*}}}*/

    /*----------------------------attr-----------------------*/
    READ_U2(attrCount, ptr);/*{{{*/
    //printf("attr_count:%d\n", attr_count);

    for (; attrCount != 0; attrCount--)
    {
        u2 attr_name_idx;
        u4 attr_length;
        char* attr_name;
        READ_INDEX(attr_name_idx, ptr);
        READ_U4(attr_length, ptr);
        ptr += attr_length;
    }

    /*}}}*/

    /*resolve super*/
    if (super_classidx)
    {
        //printf("superidx:%d\n", super_classidx);
        classblock->super = (C)resolveClass(class, super_classidx);
    }
    return class;
    /*}}}*/
}

/*
 * Copy the Class file to the mem.Then invoke the defineClass()
 * to establish the data structure.
 *
 * invoke by : loadClass()
 * @qcliu 2015/01/27
 **/
static C loadSystemClass(char* classname)
{
    /*{{{*/
    C class = findClass(classname);
    if (class != NULL)
    {
        ClassBlock* cb = CLASS_CB(class);

        if (dis_testinfo)
            printf("%s have already in table!!!!!!!!!!!!!!!!!!!!\n",cb->this_classname);

        return class;
    }

    if (dis_testinfo)
        printf("loadSystemClass----------------------------\n");

    int file_len, fname_len = strlen(classname) + 8;
    char* buff = (char*)malloc(MAX_PATH_LEN + file_len);
    char filename[fname_len];
    char* data;
    FILE* cfd;

    filename[0] = '/';
    strcat(strcpy(&filename[1], classname), ".class");

    //printf("%s\n", filename);

    char** cp_ptr;
    for (cp_ptr = CLASSPATH; 
                *cp_ptr&&!(cfd = fopen(strcat(strcpy(buff, *cp_ptr), filename), "r"));
                    cp_ptr++)
                ;
    if (cfd == NULL)
    {
        cfd = fopen(strcat(strcpy(buff, PREFIX), filename), "r");
    }
    if (dis_testinfo)
        printf("this fiel name is %s\n", filename);

    if (cfd == NULL)
    {
        printf("\n%s\n", buff);
        printf("\n%s\n", classname);
        throwException("NoFileInput");
    }

    fseek(cfd, 0L, SEEK_END);
    file_len = ftell(cfd);
    fseek(cfd, 0L, SEEK_SET);

    data = (char*)sysMalloc(file_len);
    if (fread(data, sizeof(char), file_len, cfd) != file_len)
    {
        printf("\n%s\n", classname);
        throwException("NoFileInput");
    }
    else
    {
        /*defineClass()*/
        class = defineClass(classname, data, file_len);
    }

    free(buff);
    free(data);



    return class;
    /*}}}*/
}

/*
 * Obtain the methods_table_size of each ClassBlock
 * During this function,<init>,<clinit>,static,private methods can
 * be omit.The override can be given the index according to super_class.
 *
 * invoke by: loadClass()
 **/
static void prepareClass(C class)
{
    /*{{{*/
    ClassBlock* cb = CLASS_CB(class);
    u2 this_methodstable_size = 0;
    u2 super_methodstable_size = 0;
    int obj_size = 0;
    /*
     * The offset of filed of object that in javaheap
     * note: start from 1 !!!!!! Take 0 as the failure case.
     * @qcliu 2015/01/30
     */
    int field_offset = 1;
    int loop = 0;

    //if already prepared,return
    if (cb->flags >= PREPARED)
        return;

    if (dis_testinfo)
        printf("preparing class.....%s\n", cb->this_classname);

    //has super
    if (cb->super != NULL)
    {
        int i = 0;
        u2 idx = 0;
        ClassBlock* super_cb = CLASS_CB(cb->super);
        super_methodstable_size = super_cb->methods_table_size;
        /*
         * Extends super's filed, for prepare field.
         * note: Just like the init field_offset=1, this field_offset
         *       also take 0 as the failure case.
         * @qcliu 2015/01/30
         */
        field_offset = super_cb->obj_size + 1;


        //prepare methods
        //table driven approache
        for (i = 0; i<cb->methods_count; i++)
        {
            MethodBlock* mb = &cb->methods[i];

            //omit static,private,clinit,init
            if ((mb->access_flags & ACC_STATIC) || (mb->access_flags & ACC_PRIVATE) ||
                    (strcmp(mb->name, "<init>") == 0) || (strcmp(mb->name, "<clinit>") == 0))
                continue;

            /*the findMethod() is return the same method which is as elder as
             *possible in super classes*/
            MethodBlock* super_mb = findMethod(cb->super, mb->name, mb->type);
            if (super_mb)
            {
                //override
                mb->methods_table_idx = super_mb->methods_table_idx;
            }
            else
            {
                /*
                 ----------------------
                 |                     |
                 |                     |
                 |super_methods_table  |
                 |                     |
                 ----------------------
                 |                     |
                 ----------------------
                 |                     |
                 ----------------------
                 |

                 */
                mb->methods_table_idx = super_methodstable_size + idx++;

            }

        }//end for(i = 0; i<cb->methods_count; i++)
        cb->methods_table_size = super_methodstable_size + idx;
    }
    else  //Object.class
    {
        int i = 0;
        int idx = 0;

        for (i = 0; i<cb->methods_count; i++)
        {
            MethodBlock* mb = &cb->methods[i];
            /* omit <init>,<clinit>,static,private */
            if ((strcmp(mb->name, "<init>") == 0) || (strcmp(mb->name, "<clinit>") == 0) ||
                    (mb->access_flags & ACC_STATIC) || (mb->access_flags & ACC_PRIVATE))
                continue;
            this_methodstable_size++;
            /*determine the final index*/
            mb->methods_table_idx = idx++;
        }
        cb->methods_table_size = this_methodstable_size;
    }//end if (cb->super != NULL)


    //prepare field
    /*
     * calculate erery field's offset that in the object moudle in javaheap.
     * for static variable no need a offset,but need give them initial value
     */
    for(loop = 0; loop<cb->fields_count; loop++)
    {
        int i = loop;
        FieldBlock* fb = &cb->fields[i];

        //static field
        if (fb->access_flags & ACC_STATIC)
        {
            if ((strcmp(fb->type, "J") == 0) || (strcmp(fb->type, "D") == 0))
                *(long long*)(&fb->static_value) = 0;
            else
                fb->static_value = 0;
        }
        else   //non static
        {
            /*
             * note: the filed_offset now is already become the super offset
             */
            fb->offset = field_offset;
            if ((strcmp(fb->type, "J") == 0) || (strcmp(fb->type, "D") == 0))
                field_offset += 2;
            else
                field_offset += 1;
        }
    }
    /*
     * note: Since the filed_offset is init by 1, so the real size is offset-1.
     * @qcliu 2015/01/30
     */
    cb->obj_size = field_offset - 1;



    cb->flags = PREPARED;
    /*}}}*/
}//end prepareClass()



/*
 * Now,every ClassBlocks have had  a specific methods_table_size accoring to preparClass().
 * merge the table!!!!!
 *
 * invoke by: loadClass()
 */

static C linkClass(C class)
{
    /*{{{*/
    ClassBlock* cb = CLASS_CB(class);

    if(cb->access_flags & ACC_INTERFACE)
        return class;

    if (cb->flags >= LINKED)
        return class;

    int this_methodstable_size = cb->methods_table_size;
    int super_methodstable_size = 0;

    if (dis_testinfo)
        printf("linking class......%s\n",cb->this_classname);

    if (cb->super != NULL)  //has super
    {
        int i;
        ClassBlock* super_cb = CLASS_CB(cb->super);
        super_methodstable_size = super_cb->methods_table_size;
        cb->methods_table = (MethodBlock**)sysMalloc(sizeof(MethodBlock*)*
                            this_methodstable_size);
        memcpy(cb->methods_table, super_cb->methods_table, super_methodstable_size * (sizeof(MethodBlock*)));

        for (i = 0; i<cb->methods_count; i++)
        {
            MethodBlock* mb = &cb->methods[i];


            if ((strcmp(mb->name, "<clinit>") == 0) || (strcmp(mb->name, "<init>") == 0) ||
                    (mb->access_flags & ACC_STATIC) || (mb->access_flags & ACC_PRIVATE))
                continue;

            u2 idx = mb->methods_table_idx;
            cb->methods_table[idx] = mb;
        }
    }
    else  //Object.class
    {
        int i = 0;

        cb->methods_table = (MethodBlock**)sysMalloc(sizeof(MethodBlock*) * this_methodstable_size);

        for (i = 0; i<cb->methods_count; i++)
        {
            MethodBlock* mb = &cb->methods[i];
            if ((strcmp(mb->name, "<clinit>") == 0) || (strcmp(mb->name, "<init>") == 0) ||
                    (mb->access_flags & ACC_STATIC) || (mb->access_flags & ACC_PRIVATE))
                continue;
            u2 idx = mb->methods_table_idx;
            cb->methods_table[idx] = mb;
        }
    }//end if(cd->super != NULL)


    /*in the <clinit>, it maybe need java/lang/Class*/


    if (java_lang_Class)
    {
        O obj = allocObject(java_lang_Class);

        if (java_lang_VMClass == NULL)
            java_lang_VMClass = loadClass("java/lang/VMClass");

        O vmobj = allocObject(java_lang_VMClass);
        //NOTE: vmClass obj can also known which class he belong to
        vmobj->binding = (C)obj;

        /* Every Class obj need a VMClass obj.*/
        FieldBlock* fb = findField(java_lang_Class, "vmClass", "Ljava/lang/VMClass;");
        OBJECT_DATA(obj, fb->offset-1, O) = vmobj;

        obj->binding = class;//Class obj's binding refer to the method area.
        class->class = obj;
    }

    cb->flags = LINKED;

    return class;
    /*}}}*/
}//end linkClass()

/*
 * This is to invoke the class's <clinit>
 *
 * invoke by: loadClass()
 * @qcliu 2015/01/27
 */
void initClass(C class)
{
    /*{{{*/
    if (!initable)
        return;

    ClassBlock* cb = CLASS_CB(class);

    if (dis_testinfo)
        printf("initClass-------------%s\n", cb->this_classname);

    if (cb->flags >= INITED)
    {
        if (dis_testinfo)
            printf("\nalreay inited.\n\n");

        return;
    }

    MethodBlock* mb = findMethodinCurrent(class, "<clinit>", "()V");
    /*
     * The <clinit> is a static method, it's dosen't refer to
     * args copy. So, although the staticmain has not executed,
     * <clinit> can work safety.
     * @qcliu 2015/01/30
     */
    if (mb)
        executeMethod(mb, NULL);
    else
    {
        if (dis_testinfo)
            printf("not find <clinit>\n");
    }
    cb->flags = INITED;
    /*}}}*/
}


C loadClass_not_init(char* classname)
{
    /*{{{*/
    C class = findClass(classname);
    ClassBlock* cb;

    if (class != NULL)
    {
        ClassBlock* cb = CLASS_CB(class);

        if (dis_testinfo)
            printf("%s have already in the table!!!!!!!!!!!!!!!\n",cb->this_classname);

        return class;
    }
    else if (classname[0] == '[')
        class = loadArrayClass(classname);
    else
        class = loadSystemClass(classname);

    /*
     * This must before initClass().Otherwise, there will
     * be loop untill death.
     **/
    //add class to the list
    //List_addLast(CList, class);
    cb = CLASS_CB(class);
    String_t s = String_new(cb->this_classname);
    Hash_put(CMap, s, class);


    //prepareClass
    prepareClass(class);

    //linkClass
    class = linkClass(class);

    // If the class is not interface, initClass
    cb = CLASS_CB(class);

    //if (dis_testinfo)
    //  printList(head);

    return class;
    /*}}}*/

}
/*
 * First find class in LinkedList, if not, there need a assert
 * weather it's a array or a normal class.
 * When loaded the class, need prepareClass() and linkClass().
 * The recursive method loadSystemClass() ensured that the super
 * class is already loaded. So the prepareClass() and linkClass()
 * can execute.
 *
 * invoke by: vm.c, resolveClass()
 * @qcliu 2015/01/27
 */
C loadClass(char* classname)
{
    /*{{{*/
    C class = findClass(classname);
    ClassBlock* cb;

    if (class != NULL)
    {
        ClassBlock* cb = CLASS_CB(class);

        if (dis_testinfo)
            printf("%s have already in the table!!!!!!!!!!!!!!!\n",cb->this_classname);

        return class;
    }
    else if (classname[0] == '[')
        class = loadArrayClass(classname);
    else
        class = loadSystemClass(classname);

    /*
     * This must before initClass().Otherwise, there will
     * be loop untill death.
     **/
    //add class to the list
    //List_addLast(CList, class);
    cb = CLASS_CB(class);
    String_t s = String_new(cb->this_classname);
    Hash_put(CMap, s, class);


    //prepareClass
    prepareClass(class);

    //linkClass
    class = linkClass(class);

    // If the class is not interface, initClass
    cb = CLASS_CB(class);

    if (!(cb->access_flags & ACC_INTERFACE))
        initClass(class);


    //if (dis_testinfo)
    //  printList(head);

    return class;
    /*}}}*/
}


/**
  * This is a recursive for multi-array, normal-array is a special case.
  * If it's a multi-array,there will be loadClass() for it's cb->element.
  * The end would be a reference type, or a array whitch is primitive
  * type, both of them will end the recursive.
  */

static C loadArrayClass(char* classname)
{
    /*{{{*/
    //printf("%s, %d\n", classname, strlen(classname));
    int len = strlen(classname);
    int size = ClassHeaderSize+sizeof(ClassBlock);
    //C class = (C)sysMalloc(sizeof(Class) + sizeof(ClassBlock));
    C class = (C)sysMalloc(size);
    ClassBlock* cb = CLASS_CB(class);

    cb->flags = 0;
    cb->access_flags = 0;
    cb->methods_table = NULL;
    cb->element = NULL;
    cb->dim = 0;
    cb->super = NULL;
    cb->this_classname = classname;
    //necessary
    cb->fields_count = 0;
    cb->methods_count = 0;
    cb->dim = 0;
    cb->type_flags = 0;
    cb->element = NULL;

    cb->this_classname = strcpy((char*)sysMalloc(len + 1), classname);
    cb->super = loadClass("java/lang/Object");

    cb->interface_count = 2;

    if (dis_testinfo)
        printf("this array is %s\n",cb->this_classname);

    cb->interfaces = (C*)sysMalloc(2 * sizeof(C));
    cb->interfaces[0] = loadClass("java/lang/Cloneable");
    cb->interfaces[1] = loadClass("java/io/Serializable");

    //according to the classname, determining dim, element
    if (classname[1] == '[')  //multi array
    {
        cb->element = loadClass(classname + 1);

        ClassBlock* temp = CLASS_CB(cb->element);
        cb->dim = temp->dim + 1;
    }
    else  //reaching the last dim
    {
        if (classname[len -1] == ';')
        {
            //get the name of reference type, omit the last ';'
            //and start '[L'
            char* newname = (char*)sysMalloc(len - 2);
            memcpy(newname, classname+2, len-3);
            newname[len-3] = '\0';

            //this must be a reference type
            cb->element = loadClass(newname);
        }
        else
        {
            //primitivte type
            cb->element = findPrimitiveClass(classname[len - 1]);
            //cb->flags = PRIM;
        }

        cb->dim += 1;
    }

    if (java_lang_Class)
    {
        O obj = allocObject(java_lang_Class);
        obj->binding = class;
        class->class = obj;
    }

    return class;
    /*}}}*/
}

/*
 * first findClass in LinedList, if not find, load the
 * arrayClass.
 */
C findArrayClass(char* classname)
{
    /*{{{*/
    C class = findClass(classname);

    if (class != NULL)
        return class;
    else
        class = loadClass(classname);

    ClassBlock* cb = CLASS_CB(class);
    cb->type_flags = ARRAY;


    return class;
    /*}}}*/
}


/* This function is to load primitive class
 * int ,char, short, long, double, float, boolean.
 *
 * NOTE: has super??? need prepareClass() and linkClass()???
 * @qcliu 2015/03/28
 * invoked by: findPrimitiveClass()
 */
static C loadPrimitiveClass(char* classname, int index)
{
    /*{{{*/
    int len = strlen(classname);
    int size = ClassHeaderSize+sizeof(ClassBlock);
    //C class = (C)sysMalloc(sizeof(Class) + sizeof(ClassBlock));
    C class = (C)sysMalloc(size);
    ClassBlock* cb = CLASS_CB(class);

    cb->flags = 0;
    cb->access_flags = 0;
    cb->methods_table = NULL;
    cb->element = NULL;
    cb->dim = 0;
    cb->super = NULL;
    cb->this_classname = classname;
    //necessary
    cb->fields_count = 0;
    cb->methods_count = 0;
    cb->interface_count = 0;
    cb->interfaces = NULL;
    cb->type_flags = 0;

    cb->this_classname = strcpy((char*)sysMalloc(len + 1), classname);
    //cb->super = loadClass("java/lang/Object");

    cb->element = NULL;
    cb->dim = 0;

    //List_addLast(CList, class);
    String_t s = String_new(cb->this_classname);
    Hash_put(CMap, s, class);
    //prepareClass
    prepareClass(class);
    //linkClass
    class = linkClass(class);

    cb->flags = PRIM;
    /*
        if (java_lang_Class)
        {
            Object* obj = allocObject(java_lang_Class);
            obj->binding = class;
            class->class = obj;
        }
    */
    primClass[index] = class;
    return class;

    /*}}}*/
}

/*Find Primitive Class
 *@qcliu 2015/03/20
 */
C findPrimitiveClass(char primtype)
{
    /*{{{*/
    int index;
    C primclass;
    char* classname;
    switch (primtype)
    {
    case 'B':
        index = 0;
        classname = "B";
        break;
    case 'S':
        classname = "S";
        index = 1;
        break;
    case 'I':
        classname = "I";
        index = 2;
        break;
    case 'J':
        classname = "J";
        index = 3;
        break;
    case 'C':
        classname = "C";
        index = 4;
        break;
    case 'F':
        classname = "F";
        index = 5;
        break;
    case 'D':
        classname = "D";
        index = 6;
        break;
    case 'Z':
        classname = "Z";
        index = 7;
        break;
    default:
        throwException("findPrimitiveClass() error");
    }


    if(primClass[index] != NULL)
        return primClass[index];
    else
        return loadPrimitiveClass(classname, index);
    /*}}}*/
}

char* getClassPath()
{
    return getenv("CLASSPATH");
}

#undef C
#undef O
#undef P
