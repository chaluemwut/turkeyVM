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

#define LOAD(f, v, t, i)        \
    do{                         \
        v = *(t*)(f->locals+i);  \
    }while(0)

#define STORE(f, v, t, i)       \
    do{                         \
        *(t*)(f->locals+i)=v;   \
    }while(0)

typedef enum ntype {
    TYPE_INT,
    TYPE_LONG,
    TYPE_ULONG,
    TYPE_DOUBLE,
    TYPE_CHAR,
    TYPE_UINT,
    TYPE_REFERENCE,
    TYPE_FLOAT
}Type;

typedef struct NF *NF;
typedef struct JF *JF;

struct NF
{
    MethodBlock* mb;
    C class;
    unsigned int* locals;
    //NF prev;
};


struct JF{
    MethodBlock* mb;//current_method
    unsigned char* pc;
    ConstantPool* cp;//current_cp
    C class;//current_class
    //ostack, local
    unsigned int* ostack;
    unsigned int* locals;
    unsigned int* bottom;
    unsigned int* top;
    //JF prev;//point to the previous Frame
    int pc_offset;

    u8 ret;
    //for test
    int id;
};



//extern NativeFrame* nframe;
//extern Frame* current_frame;

extern int getNewId();

extern JF initFrame();

extern void initNativeFrame();

extern int getCurrentFrameId();

extern JF getCurrentFrame();

extern ConstantPool* getCurrentCP();

extern unsigned  char* getCurrentPC();

extern char* getCurrentMethodName();

extern char* getCurrentMethodDesc();

extern unsigned int getCurrentPCOffset();


extern void PCIncrease(int x);

extern void PCDecrease(int x);

extern unsigned int getCurrentCodeLen();

extern C getCurrentClass();

extern void setCurrentFrame(JF f);

extern JF popFrame();

extern JF createFrame0(MethodBlock* mb);

extern JF createFrame(MethodBlock* mb, va_list jargs, void* ret);

extern void popNativeFrame();

extern void createNativeFrame(MethodBlock* mb);

extern NF getNativeFrame();

extern void pop(JF f, void* result, Type t);

extern void push(JF f, void* value, Type t);

extern void load(void* result, Type t, int index);

extern void store(void* value, Type t, int index);


void print_Stack(JF frame);


#undef C
#undef JF
#undef NF
#endif
