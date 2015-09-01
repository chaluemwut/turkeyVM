#ifndef STACKMANAGER_H
#define STACKMANAGER_H

#include "../main/turkey.h"
#include <stdarg.h>

#define C Class_t
#define JF JFrame_t
#define NF NFrame_t

#define SCAN_SIG(p, D, S)			            \
    do {                                        \
        p++;               /* skip start ( */	\
        while(*p != ')') {				        \
            if((*p == 'J') || (*p == 'D')) {    \
                D;					            \
                p++;					        \
            } else {					        \
                S;					            \
                if(*p == '[')				    \
                for(p++; *p == '['; p++);		\
                if(*p == 'L')				    \
                while(*p++ != ';');	            \
                else                            \
                p++;                            \
            }                                   \
        }                                       \
        p++; /* skip end ) */                   \
    }while(0)

#define VA_DOUBLE(args, sp) *(u8*)sp = va_arg(args, u8);sp+=2
#define VA_SINGLE(args, sp) *sp = va_arg(args, u4);sp+=1

/**
 * @f current frame
 * @v the value we want load to
 * @t type
 * @i index of frame's locals
 */
#define LOAD(f, v, t, i)                \
    do{                                 \
        v = *(t*)(f->locals+(i));       \
    }while(0)

#define STORE(f, v, t, i)               \
    do{                                 \
        *(t*)(f->locals+(i))=v;         \
    }while(0)

#define GET_CONSTANTPOOL(f) (f->cp)
#define GET_PC(f)   (f->pc)
#define GET_CLASS(f)    (f->class)
#define GET_OFFSET(f)   (f->pc_offset)

typedef enum {
    TYPE_INT,
    TYPE_LONG,
    TYPE_ULONG,
    TYPE_DOUBLE,
    TYPE_CHAR,
    TYPE_UINT,
    TYPE_REFERENCE,
    TYPE_FLOAT
} Operand_e;

typedef struct NF *NF;
typedef struct JF *JF;

struct NF {
    MethodBlock_t *mb;
    C class;
    unsigned int *locals;
    //NF prev;
};

struct JF {
    MethodBlock_t *mb;          //current_method
    unsigned char *pc;
    ConstantPool_t *cp;         //current_cp
    C class;                    //current_class
    //ostack, local
    unsigned int *ostack;
    unsigned int *locals;
    unsigned int *bottom;
    unsigned int *top;
    //JF prev;//point to the previous Frame
    int pc_offset;

    u8 ret;
    //for test
    int id;
};

extern int getNewId();

extern JF initFrame();

extern void initNativeFrame();

extern JF getCurrentFrame();

extern void setCurrentFrame(JF f);

extern JF popFrame();

extern JF createFrame0(MethodBlock_t * mb);

extern JF createFrame(MethodBlock_t * mb, va_list jargs, void *ret);

extern void popNativeFrame();

extern void createNativeFrame(MethodBlock_t * mb);

extern NF getNativeFrame();

extern void pop(JF f, void *result, Operand_e t);

extern void push(JF f, void *value, Operand_e t);

//extern void load(void* result, Operand_e t, int index);

//extern void store(void* value, Operand_e t, int index);

#undef C
#undef JF
#undef NF
#endif
