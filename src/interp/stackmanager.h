#ifndef STACK_H
#define STACK_H

#include "../main/vm.h"
#include <stdarg.h>
#define C Class_t
#define SCAN_SIG(p, D, S)			\
   p++;               /* skip start ( */	\
   while(*p != ')') {				\
       if((*p == 'J') || (*p == 'D')) {		\
          D;					\
          p++;					\
      } else {					\
          S;					\
          if(*p == '[')				\
              for(p++; *p == '['; p++);		\
          if(*p == 'L')				\
              while(*p++ != ';');		\
          else					\
              p++;				\
      }						\
   }						\
   p++;               /* skip end ) */

#define VA_DOUBLE(args, sp) *(u8*)sp = va_arg(args, u8);sp+=2
#define VA_SINGLE(args, sp) *sp = va_arg(args, u4);sp+=1

typedef enum type {
    TYPE_INT,
    TYPE_LONG,
    TYPE_ULONG,
    TYPE_DOUBLE,
    TYPE_CHAR,
    TYPE_UINT,
    TYPE_REFERENCE,
    TYPE_FLOAT
}Type;

typedef struct native_frame
{
    MethodBlock* mb;
    C class;
    unsigned int* locals;
    struct native_frame* prev;
}NativeFrame;

typedef struct frame
{
    MethodBlock* mb;//current_method
    unsigned char* pc;
    ConstantPool* cp;//current_cp
    C class;//current_class
    //ostack, local
    unsigned int* ostack;
    unsigned int* locals;
    unsigned int* bottom;
    unsigned int* top;
    struct frame* prev;//point to the previous Frame
    int pc_offset;

    u8 ret;
    //for test
    int id;
}Frame;



//extern NativeFrame* nframe;
//extern Frame* current_frame;

extern int getNewId();

extern Frame* initFrame();

extern void initNativeFrame();

extern int getCurrentFrameId();

extern Frame* getCurrentFrame();

extern ConstantPool* getCurrentCP();

extern unsigned  char* getCurrentPC();

extern char* getCurrentMethodName();

extern char* getCurrentMethodDesc();

extern unsigned int getCurrentPCOffset();


extern void PCIncrease(int x);

extern void PCDecrease(int x);

extern unsigned int getCurrentCodeLen();

extern C getCurrentClass();

extern void setCurrentFrame(Frame* f);

extern void popFrame();

extern Frame* createFrame0(MethodBlock* mb);

extern void createFrame(MethodBlock* mb, va_list jargs, void* ret);

extern void popNativeFrame();

extern void createNativeFrame(MethodBlock* mb);

extern NativeFrame* getNativeFrame();



extern void pop(void* result, Type t);

extern void push(void* value, Type t);

extern void load(void* result, Type t, int index);

extern void store(void* value, Type t, int index);

extern void returnValue(void* ret, Type t);

#undef C
#endif
