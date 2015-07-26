#ifndef VM_H
#define VM_H
#include <stdarg.h>
//#include "../lib/list.h"
#include "../lib/hash.h"
#include "../heapManager/alloc.h"

#define FALSE 0
#define TRUE 1

#define DEBUG(format, ...) printf("FILE: "__FILE__", LINE: %d: "format"\n",__LINE__, ##__VA_ARGS__);

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
#define CLASS_CB(classref) (ClassBlock*)(classref+1)
#define CP_TYPE(cp,i) cp->type[i]
#define CP_INFO(cp,i) cp->info[i]
#define READ_INDEX(v,p) READ_U2(v,p)
#define CP_UTF8(cp,i) (char*)(cp->info[i]) 

#define IS_INTERFACE(cb)  (cb->access_flags&ACC_INTERFACE)
#define IS_ARRAY(cb)     (cb->type_flags>=ARRAY)
#define IS_PRIM(cb)      (cb->type_flags>=PRIM)

#define INST_DATA(objectRef) ((u4*)(objectRef+1))
#define OBJECT_DATA(obj, index, type) *((type*)(obj->data+index))
#define ARRAY_DATA(obj, index, type) *((type*)obj->data+index)

/*op code*/
#define OPC_NOP				0/*{{{*/
#define OPC_ACONST_NULL			1
#define OPC_ICONST_M1			2
#define OPC_ICONST_0			3
#define OPC_ICONST_1			4
#define OPC_ICONST_2			5
#define OPC_ICONST_3			6
#define OPC_ICONST_4			7
#define OPC_ICONST_5			8
#define OPC_LCONST_0			9
#define OPC_LCONST_1			10
#define OPC_FCONST_0			11
#define OPC_FCONST_1			12
#define OPC_FCONST_2			13
#define OPC_DCONST_0			14
#define OPC_DCONST_1			15
#define OPC_BIPUSH			16
#define OPC_SIPUSH			17
#define OPC_LDC				18
#define OPC_LDC_W			19
#define OPC_LDC2_W			20
#define OPC_ILOAD			21
#define OPC_LLOAD			22
#define OPC_FLOAD			23
#define OPC_DLOAD			24
#define OPC_ALOAD			25
#define OPC_ILOAD_0			26
#define OPC_ILOAD_1			27
#define OPC_ILOAD_2			28
#define OPC_ILOAD_3			29
#define OPC_LLOAD_0			30
#define OPC_LLOAD_1			31
#define OPC_LLOAD_2			32
#define OPC_LLOAD_3			33
#define OPC_FLOAD_0			34
#define OPC_FLOAD_1			35
#define OPC_FLOAD_2			36
#define OPC_FLOAD_3			37
#define OPC_DLOAD_0			38
#define OPC_DLOAD_1			39
#define OPC_DLOAD_2			40
#define OPC_DLOAD_3			41
#define OPC_ALOAD_0			42
#define OPC_ALOAD_1			43
#define OPC_ALOAD_2			44
#define OPC_ALOAD_3			45
#define OPC_IALOAD			46
#define OPC_LALOAD			47
#define OPC_FALOAD			48
#define OPC_DALOAD			49
#define OPC_AALOAD			50
#define OPC_BALOAD			51
#define OPC_CALOAD			52
#define OPC_SALOAD			53
#define OPC_ISTORE			54
#define	OPC_LSTORE			55
#define	OPC_FSTORE			56
#define	OPC_DSTORE			57
#define OPC_ASTORE			58
#define OPC_ISTORE_0			59
#define OPC_ISTORE_1			60
#define OPC_ISTORE_2			61
#define OPC_ISTORE_3			62
#define OPC_LSTORE_0			63
#define OPC_LSTORE_1			64
#define OPC_LSTORE_2			65
#define OPC_LSTORE_3			66
#define OPC_FSTORE_0			67
#define OPC_FSTORE_1			68
#define OPC_FSTORE_2			69
#define OPC_FSTORE_3			70
#define OPC_DSTORE_0			71
#define OPC_DSTORE_1			72
#define OPC_DSTORE_2			73
#define OPC_DSTORE_3			74
#define OPC_ASTORE_0			75
#define OPC_ASTORE_1			76
#define OPC_ASTORE_2			77
#define OPC_ASTORE_3			78
#define OPC_IASTORE			79
#define OPC_LASTORE			80
#define OPC_FASTORE			81
#define OPC_DASTORE			82
#define OPC_AASTORE			83
#define OPC_BASTORE			84
#define OPC_CASTORE			85
#define OPC_SASTORE			86
#define OPC_POP				87
#define OPC_POP2			88
#define OPC_DUP				89
#define OPC_DUP_X1			90
#define OPC_DUP_X2			91
#define OPC_DUP2			92
#define OPC_DUP2_X1			93
#define OPC_DUP2_X2			94
#define OPC_SWAP			95
#define OPC_IADD			96
#define OPC_LADD			97
#define OPC_FADD			98
#define OPC_DADD			99
#define OPC_ISUB			100
#define OPC_LSUB			101
#define OPC_FSUB			102
#define OPC_DSUB			103
#define OPC_IMUL			104
#define OPC_LMUL			105
#define OPC_FMUL			106
#define OPC_DMUL			107
#define OPC_IDIV			108
#define OPC_LDIV			109
#define OPC_FDIV			110
#define OPC_DDIV			111
#define OPC_IREM			112
#define OPC_LREM			113
#define OPC_FREM			114
#define OPC_DREM			115
#define OPC_INEG			116
#define OPC_LNEG			117
#define OPC_FNEG			118
#define OPC_DNEG			119
#define OPC_ISHL			120
#define OPC_LSHL			121
#define OPC_ISHR			122
#define OPC_LSHR			123
#define OPC_IUSHR			124
#define OPC_LUSHR			125
#define OPC_IAND			126
#define OPC_LAND			127
#define OPC_IOR				128
#define OPC_LOR				129
#define OPC_IXOR			130
#define OPC_LXOR			131
#define OPC_IINC			132
#define OPC_I2L				133
#define OPC_I2F				134
#define OPC_I2D				135
#define OPC_L2I				136
#define OPC_L2F             137
#define OPC_L2D				138
#define OPC_F2I				139
#define OPC_F2L				140
#define OPC_F2D				141
#define OPC_D2I				142
#define OPC_D2L				143
#define OPC_D2F				144
#define OPC_I2B				145
#define OPC_I2C				146
#define OPC_I2S				147
#define OPC_LCMP			148
#define OPC_FCMPL			149
#define OPC_FCMPG			150
#define OPC_DCMPL			151
#define OPC_DCMPG			152
#define OPC_IFEQ			153
#define OPC_IFNE			154
#define OPC_IFLT			155
#define OPC_IFGE			156
#define OPC_IFGT			157
#define OPC_IFLE			158
#define OPC_IF_ICMPEQ			159
#define OPC_IF_ICMPNE			160
#define OPC_IF_ICMPLT			161
#define OPC_IF_ICMPGE			162
#define OPC_IF_ICMPGT			163
#define OPC_IF_ICMPLE			164
#define OPC_IF_ACMPEQ			165
#define OPC_IF_ACMPNE			166
#define OPC_GOTO			167
#define OPC_JSR                         168
#define OPC_RET                         169
#define OPC_TABLESWITCH			170
#define OPC_LOOKUPSWITCH		171
#define OPC_IRETURN			172
#define OPC_LRETURN			173
#define OPC_FRETURN			174
#define OPC_DRETURN			175
#define OPC_ARETURN			176
#define OPC_RETURN			177
#define OPC_GETSTATIC			178
#define OPC_PUTSTATIC			179
#define OPC_GETFIELD			180
#define OPC_PUTFIELD			181
#define OPC_INVOKEVIRTUAL		182
#define OPC_INVOKESPECIAL		183
#define OPC_INVOKESTATIC		184
#define OPC_INVOKEINTERFACE		185
#define OPC_INVOKEDYYNAMIC      186
#define OPC_NEW				187
#define OPC_NEWARRAY			188
#define OPC_ANEWARRAY			189
#define OPC_ARRAYLENGTH			190
#define OPC_ATHROW			191
#define OPC_CHECKCAST			192
#define OPC_INSTANCEOF			193
#define OPC_MONITORENTER		194
#define OPC_MONITOREXIT			195
#define OPC_WIDE			196
#define OPC_MULTIANEWARRAY		197
#define OPC_IFNULL			198
#define OPC_IFNONNULL			199
#define OPC_GOTO_W			200
#define OPC_JSR_W			201/*}}}*/

/*constant pool*/
#define CONSTANT_Utf8 1/*{{{*/
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
#define ACC_PUBLIC   0x0001/*{{{*/
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
#define ACC_ENUM    0x4000/*}}}*/

/*method*/


#define C Class_t
#define O Object_t

typedef struct C *C;


typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;
typedef unsigned long long  u8;
typedef struct classblock ClassBlock;


/* Every Class has a head, whitch is an java/lang/Class's 
 * Object.
 * The object's class point to the MethodsArea which happen in
 * getClass().
 *
 * @qcliu 2015/03/21
 */
struct C
{
    struct Object_t* class;
};



typedef u4 ConstantPoolEntry;

typedef struct constant_pool
{
    volatile u1* type;
    ConstantPoolEntry* info;

}ConstantPool;

typedef struct fieldblock
{
    u2 access_flags;
    char* name;
    char* type;
    u2 constant;//static final
    int static_value;//static
    int offset;//if not static, it has a offset to mark the allocation in object
}FieldBlock;

typedef struct code_exception
{
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type;
}CodeException;

typedef struct methodblock
{
    u2 access_flags;
    char* name;
    char* type;
    //attr_code
    u2 max_stack;
    u2 max_locals;
    u4 code_length;
    u1* code;
    u2 exception_table_length;
    CodeException* code_exception;
    u2 code_attr_count;
    //attr_exception
    u2 number_of_exceptions;
    u2* exception_idx_table;

    u2 methods_table_idx;
    C class;
    int args_count;// this is the arg length in the stack
    u2 slot;//the offset in the ClassBlock's MethodBlock**

    void* native_invoker;//it's need bingding to the nativemethod.
    //void* native_arg;//no need

}MethodBlock;

struct classblock
{
    u4 magic;
    u2 minor_version;
    u2 major_version;
    u2 constant_pool_count;
    ConstantPool constant_pool;
    u2 access_flags;
    char* this_classname;
    char* super_classname;
    u2 interface_count;
    C* interfaces;//interface_table
    u2 fields_count;
    FieldBlock* fields;
    u2 methods_count;
    MethodBlock* methods;
    C super;//the superclass address
    MethodBlock** methods_table;
    int methods_table_size;
    u2 flags;//loaded, prepared, linked, inited...
    u2 type_flags;//array, interface

    //---------for array----------------------
    int obj_size;
    C element;
    int dim;//if it's a array, mark the division
};

extern C java_lang_Class;
extern C java_lang_VMClass;
//extern List_t CList;
//extern List_t DList;
extern Hash_t CMap;
extern Hash_t DMap;
/*--------------------------Function prototypes---------------*/

extern void exitVM();

#undef C
#undef O
#endif
