#ifndef VM_H
#define VM_H
#include <stdarg.h>
#include "../lib/hash.h"
#include "../heapManager/alloc.h"

#define FALSE 0
#define TRUE 1

#define DEBUG(format, ...) \
    printf("FILE: "__FILE__", LINE: %d: "format"\n",__LINE__, ##__VA_ARGS__);

#define T_BOOLEAN 4
#define T_CHAR 5
#define T_FLOAT 6
#define T_DOUBLE 7
#define T_BYTE 8
#define T_SHORT 9
#define T_INT 10
#define T_LONG 11
#define T_REFERENCE 12

/*some general operation*/
#define READ_U8(v,p) v=(p[0]<<56)|(p[1]<<48)|(p[2]<<40)|  \
                      (p[3]<<32)|(p[4]<<24)|(p[5]<<16)|  \
                        (p[6]<<8)|p[7];p+=8
#define READ_U4(v,p) v=(p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3];p+=4
#define READ_U2(v,p) v=(p[0]<<8)|p[1];p+=2
#define READ_U1(v,p) v=p[0];p++
#define CLASS_CB(classref) ((ClassBlock_t*)(classref+1))
#define CP_TYPE(cp,i) cp->type[i]
#define CP_INFO(cp,i) cp->info[i]
#define READ_INDEX(v,p) READ_U2(v,p)
#define CP_UTF8(cp,i) (char*)(cp->info[i])

#define IS_INTERFACE(cb)  (cb->access_flags&ACC_INTERFACE)
#define IS_ARRAY(cb)     (cb->type_flags>=ARRAY)
#define IS_PRIM(cb)      (cb->type_flags>=PRIM)

#define INST_DATA(objectRef) ((u4*)(objectRef+1))

/*constant pool*/
#define CONSTANT_Utf8 1         /*{{{ */
#define CONSTANT_Integer 3
#define CONSTANT_Float 4
#define CONSTANT_Long 5
#define CONSTANT_Double 6
#define CONSTANT_Class 7
#define CONSTANT_String 8
#define CONSTANT_Fieldref 9
#define CONSTANT_Methodref 10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_NameAndType 12
#define CONSTANT_MethodHandle 15
#define CONSTANT_MethodType 16
#define CONSTANT_InvokeDynamic 18

#define RESOLVED 19
#define PREPARED 20
#define LINKED   21
#define INITED   22
#define ARRAY    23
#define PRIM     24

/*}}}*/

/*access flags*/
#define ACC_PUBLIC   0x0001     /*{{{ */
#define ACC_PRIVATE 0x0002
#define ACC_PROTECTED 0x0004
#define ACC_STATIC  0x0008
#define ACC_FINAL   0x0010
#define ACC_SUPER   0x0020
#define ACC_SYNCHRONIZED 0x0020
#define ACC_VOLATILE 0x0040
#define ACC_BRIDGE 0x0040
#define ACC_TRANSIENT 0x0080
#define ACC_VARARGS 0x0080
#define ACC_NATIVE 0x0100
#define ACC_INTERFACE 0x0200
#define ACC_ABSTRACT 0x0400
#define ACC_STRICT 0x0800
#define ACC_SYNTHETIC 0x1000
#define ACC_ANNOTATION 0x2000
#define ACC_ENUM    0x4000      /*}}} */

/*method*/

#define C Class_t
#define O Object_t

typedef struct C *C;

typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;
typedef unsigned long long u8;

typedef struct classblock ClassBlock_t;

/* Every Class has a head, whitch is an java/lang/Class's 
 * Object.
 * The object's class point to the MethodsArea which happen in
 * getClass().
 *
 * @qcliu 2015/03/21
 */
struct C {
    struct Object_t *class;
};

typedef u4 ConstantPoolEntry_t;

typedef struct constant_pool {
    volatile u1 *type;
    ConstantPoolEntry_t *info;

} ConstantPool_t;

typedef struct fieldblock {
    u2 access_flags;
    char *name;
    char *type;
    //int fields_table_idx;
    u2 constant;                //static final
    int static_value;           //static
    int offset;                 //if not static, it has a offset to mark the allocation in object
} FieldBlock_t;

typedef struct code_exception {
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type;
} CodeException_t;

typedef struct methodblock {
    u2 access_flags;
    char *name;
    char *type;
    //attr_code
    u2 max_stack;
    u2 max_locals;
    u4 code_length;
    u1 *code;
    u2 exception_table_length;
    CodeException_t *code_exception;
    u2 code_attr_count;
    //attr_exception
    u2 number_of_exceptions;
    u2 *exception_idx_table;

    u2 methods_table_idx;
    C class;
    int args_count;             // this is the arg length in the stack
    u2 slot;                    //the offset in the ClassBlock's MethodBlock**

    void *native_invoker;       //it's need bingding to the nativemethod.

} MethodBlock_t;

struct classblock {
    u4 magic;
    u2 minor_version;
    u2 major_version;
    u2 constant_pool_count;
    ConstantPool_t constant_pool;
    u2 access_flags;
    char *this_classname;
    char *super_classname;
    u2 interface_count;
    C *interfaces;              //interface_table
    u2 fields_count;
    FieldBlock_t *fields;
    //FieldBlock_t** fields_table;
    //int fields_table_size;
    //int private_start;
    u2 methods_count;
    C super;                    //the superclass address
    MethodBlock_t *methods;
    MethodBlock_t **methods_table;
    int methods_table_size;
    u2 flags;                   //loaded, prepared, linked, inited...
    u2 type_flags;              //array, interface

    //---------for array----------------------
    int obj_size;
    C element;
    int dim;                    //if it's a array, mark the division
};

/*--------------------------Function prototypes---------------*/

extern void exitVM();

extern char *getMethodClassName(MethodBlock_t * mb);

#undef C
#undef O
#endif
