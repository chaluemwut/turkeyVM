/*------------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  interp.c                               */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2015/01/23                             */
/*  DESCRIPTION           :  this is interpreter for turkeyVM       */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 * 2015/01/25
 * Packing the stack operation by macros. So it's easy to use
 *
 *//*}}}*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "resolve.h"
#include "control.h"
#include "testvm.h"
#include "exception.h"
#include "vm.h"
#include "string.h"
#include "reflect.h"
// to contral the pc move
#define PCMOVE(x) pc_offset+=(x); current_frame->pc+=(x)/*{{{*/
#define PCBACK(x) pc_offset-=(x); current_frame->pc-=(x)
#define READ_INT(v, p) v=(p[0]<<24|p[1]<<16|p[2]<<8|p[3])
#define READ_IDX(v,p) v=(p[0]<<8)|p[1]
#define READ_SHORT(v,p) READ_IDX(v,p);
#define READ_BYTE(v,p) v=*p
/* ostack and locals
 * check the margin before use
 */
#define POP(value, type) \
    value = *(type*)(current_frame->ostack); \
    *(type*)(current_frame->ostack) = 0;     \
    if (current_frame->ostack < min_stack_pointer|| current_frame->ostack > max_stack_pointer) \
    {printf("stack error!!!!\n"); exit(0);} \
    current_frame->ostack--
#define POP2(value, type) \
    current_frame->ostack--;  \
    value = *(type*)(current_frame->ostack); \
    *(type*)(current_frame->ostack) = 0;     \
    if (current_frame->ostack < min_stack_pointer|| current_frame->ostack > max_stack_pointer) \
    {printf("stack error!!!!\n"); exit(0);} \
    current_frame->ostack--
#define PUSH(value, type) \
    current_frame->ostack++;     \
    if (current_frame->ostack < min_stack_pointer|| current_frame->ostack > max_stack_pointer) \
    {printf("stack error!!!!\n"); exit(0);} \
    *(type*)current_frame->ostack = value
#define PUSH2(value, type) \
    current_frame->ostack++;     \
    if (current_frame->ostack < min_stack_pointer|| current_frame->ostack > max_stack_pointer) \
    {printf("stack error!!!!\n"); exit(0);} \
    *(type*)current_frame->ostack = value;  \
    current_frame->ostack++
#define LOAD(value, type, x)    \
    if (x > max_locals)         \
    {printf("locals error\n");exit(0);}              \
    value = *(type*)(current_frame->locals+x)
#define STORE(value, type ,x)   \
    if (x > max_locals)         \
    {printf("locals error\n");exit(0);}              \
    *(type*)(current_frame->locals+x)=value/*}}}*/

//vm.h
extern Frame* current_frame;
extern LinkedList* head;

static int testnum;


/*
 * invoke by: executeMethod(), executeStaticMethod() in execute.c
 */
void executeJava() {

    // When the stack frame is move to the top, exit.
    if (current_frame->mb == NULL) {
        printf("\nframe is NULL\n");
        exitVM();
    }


    /* pc_offset is the location of the pc in the code.
     * NOTE: OPC_TABLESWITCH is use pc_offset. And pc_offset
     *      must move with the current_pc.
     *@qcliu 2015/03/16
     */
    int pc_offset = 0;
    MethodBlock* current_mb = current_frame->mb;
    unsigned int code_length = current_mb->code_length;
    //printf("%d\n", code_length);


    unsigned char* pc = current_frame->pc;
    unsigned short max_locals = current_mb->max_locals;
    unsigned short max_stack = current_mb->max_stack;

    unsigned int* min_stack_pointer = current_frame->ostack;
    unsigned int* max_stack_pointer = current_frame->ostack + max_stack;
    ClassBlock* cc = CLASS_CB(current_frame->class);

    //Native doesn't have a return
    if (0 == code_length) {
        printf("code_length == 0    ");
        ClassBlock* cb = CLASS_CB(current_frame->class);
        printf("method name:%s,type:%s, class:%s\n", current_frame->mb->name,current_frame->mb->type, cb->this_classname);

        exit(0);
       // popFrame();
        /*
         * After the popFrame(), must return executeJava().
         */
        return;
    }

        if (0 == strcmp(current_mb->name, "hashCode"))
          printf("\nhashCode\n");
        if (0 == strcmp(current_mb->name, "put"))
          printf("put");
        if (0 == strcmp(current_mb->name, "hash"))
          printf("hash");
        if (0 == strcmp(current_mb->type, "(Ljava/lang/Object;)Ljava/lang/Object;")&&0 == strcmp(current_mb->name, "get"))
          printf("get");
    if (dis_testinfo) {
        printf("current Frame is :%s, %s  locals:%d, stack:%d\n",
                    current_mb->name, current_mb->type, max_locals, max_stack);
        printf("current class:%s\n", cc->this_classname);

        if (0 == strcmp(current_mb->name, "<init>")&&(0 == strcmp(current_mb->type, "(I)V")))
          printf("\n<init>(I)V!!!!!!!!\n");
        if (0 == strcmp(current_mb->name, "getChars"))
          printf("\ngetChars\n");
        if (0 == strcmp(current_mb->name, "getEncoder"))
          printf("\ngetEncoder\n");
    }


    /*
     * note: pc_offset must move along with the current_pc. So use
     *       the PCMOVE() as much as possible.
     */
    while (pc_offset < code_length) {
        u4 c = *current_frame->pc;
        ConstantPool* current_cp = current_frame->cp;


        if (dis_bytecode)
            printf("%s, %d\n", op_code[c], ((max_stack_pointer - current_frame->ostack)));//for test

        if (dis_testinfo) {
            printf("\nopcode: ---------%s\n", op_code[c]);//for test
            printf("pc_offset:------------%d\n", pc_offset);
            printf("current_frame->pc:---------%d\n", (unsigned int)current_frame->pc);
            printStack();
        }
        switch (c) {
            case OPC_NOP://0
                break;
            case OPC_ACONST_NULL://1
                PUSH(0, int);
                break;
            case OPC_ICONST_M1://2
                PUSH(-1, int);
                break;
            case OPC_ICONST_0://3
                PUSH(0, int);
                break;
            case OPC_ICONST_1://4
                PUSH(1, int);
                break;
            case OPC_ICONST_2://5
                PUSH(2, int);
                break;
            case OPC_ICONST_3://6
                PUSH(3, int);
                break;
            case OPC_ICONST_4://7
                PUSH(4, int);
                break;
            case OPC_ICONST_5://8
                PUSH(5, int);
                break;
            case OPC_LCONST_0://9 
                {
                    PUSH2(0, long long);
                    break;
                }
            case OPC_LCONST_1: {//10
                    PUSH2(1, long long);
                    break;
                }
            case OPC_FCONST_0: {//11
                    PUSH(0, float);
                    break;
                }
            case OPC_FCONST_1: {//12
                    PUSH(1, float);
                    break;
                }
            case OPC_FCONST_2: {//13
                    PUSH(2, float);
                    break;
                }
            case OPC_DCONST_0://14
                {
                    PUSH2(0, double);
                    break;
                }
            case OPC_DCONST_1://15
                {
                    PUSH2(1, double);
                    break;
                }
            case OPC_BIPUSH://16
                {
                    int value = 0;
                    PCMOVE(1);
                    READ_BYTE(value, current_frame->pc);
                    PUSH(value, int);
                    break;
                }
            case OPC_SIPUSH://17
                {
                    //TODO
                    short value0;
                    int value;
                    PCMOVE(1);
                    READ_SHORT(value0, current_frame->pc);
                    PCMOVE(1);
                    value = (int)value0;
                    PUSH(value, int);
                    break;
                }
            case OPC_LDC://18
                {
                    //TODO
                    int index = 0;
                    u4 cp_info;
                    PCMOVE(1);
                    READ_BYTE(index, current_frame->pc);
                    cp_info = CP_INFO(current_cp, index);
                    switch (CP_TYPE(current_cp, index))
                    {
                        /*NOTE: possible???*/
                        case RESOLVED:
                            {
                                PUSH(cp_info, int);
                                break;
                            }
                        case CONSTANT_Integer:
                            {
                                int value;
                                value = (int)cp_info;
                                PUSH(value, int);
                                break;
                            }
                        case CONSTANT_Float:
                            {
                                float value;
                                /*NOTE: get the value from constants_pool as the type float
                                 *      instead of get as the type int then reverse int to float.
                                 * @qcliu 2015/03/16
                                 */
                                value = *(float*)&current_cp->info[index];
                                PUSH(value, float);
                                break;
                            }
                        case CONSTANT_String:
                            {
                                if (CP_TYPE(current_cp, cp_info) != CONSTANT_Utf8)
                                  throwException("not UTF8@ interp.c ldc");

                                /*
                                 * New a Object, which Class is java/lang/String
                                 * NOTE: should assign the value. The string is
                                 * alread in the constans-pool.
                                 * @qcliu 2015/03/08
                                 */
                                Object* obj;
                                int offset;
                                char* s = CP_UTF8(current_cp, cp_info);

                                /**/
                                obj = (Object*)createString(s);
                                if (obj == NULL)
                                  throwException("LDC error");
                                //printStringObject(obj);
                                PUSH(obj, Object*);

                                break;
                            }
                        case CONSTANT_Class:
                            {
                                Class* class = (Class*)resolveClass(current_frame->class, index);
                                if (class != NULL)
                                {
                                    Object* obj = class->class;
                                    PUSH(obj, Object*);
                                }
                                else
                                {
                                    throwException("ldc CONSTANT_Class");
                                }
                                break;
                            }
                        default:
                            throwException("LDC error");
                    }
                    break;
                }
                //TODO
            case OPC_LDC_W://19
                {
                    int index = 0;
                    u4 cp_info;
                    PCMOVE(1);
                    READ_IDX(index, current_frame->pc);
                    PCMOVE(1);

                    cp_info = CP_INFO(current_cp, index);
                    switch (CP_TYPE(current_cp, index))
                    {
                        /*NOTE: possible???*/
                        case RESOLVED:
                            {
                                PUSH(cp_info, int);
                                break;
                            }
                        case CONSTANT_Integer:
                            {
                                int value;
                                value = (int)cp_info;
                                PUSH(value, int);
                                break;
                            }
                        case CONSTANT_Float:
                            {
                                float value;
                                /*NOTE: get the value from constants_pool as the type float
                                 *      instead of get as the type int then reverse int to float.
                                 * @qcliu 2015/03/16
                                 */
                                value = *(float*)&current_cp->info[index];
                                PUSH(value, float);
                                break;
                            }
                        case CONSTANT_String:
                            {
                                if (CP_TYPE(current_cp, cp_info) != CONSTANT_Utf8)
                                  throwException("not UTF8@ interp.c ldc_w");

                                /*
                                 * New a Object, which Class is java/lang/String
                                 * NOTE: should assign the value. The string is
                                 * alread in the constans-pool.
                                 * @qcliu 2015/03/08
                                 */
                                Object* obj;
                                int offset;
                                char* s = CP_UTF8(current_cp, cp_info);

                                /**/
                                obj = (Object*)createString(s);
                                if (obj == NULL)
                                  throwException("LDC error");
                                //printStringObject(obj);
                                PUSH(obj, Object*);

                                break;
                            }
                        case CONSTANT_Class:
                            {
                                Class* class = (Class*)resolveClass(current_frame->class, index);
                                if (class != NULL) {
                                    Object* obj = class->class;
                                    PUSH(obj, Object*);
                                }
                                else {
                                    throwException("ldc CONSTANT_Class");
                                }
                                break;
                            }
                        default:
                            throwException("LDC error");
                    }

                    break;
                }
            case OPC_LDC2_W://20
                {
                    int index = 0;
                    u4 cp_info;
                    PCMOVE(1);
                    READ_IDX(index, current_frame->pc);
                    PCMOVE(1);


                    switch (CP_TYPE(current_cp, index)) {
                        case RESOLVED: {
                                //TODO
                                break;
                            }
                        case CONSTANT_Long: {
                                long long value;
                                value = *(long long*)&current_cp->info[index];
                                PUSH2(value, long long);
                                break;
                            }
                        case CONSTANT_Double: {
                                double value;
                                value = *(double*)&current_cp->info[index];
                                PUSH2(value, double);
                                break;
                            }
                        default:
                            throwException("LDC2_W error");
                    }
                    break;
                }
            case OPC_ILOAD://21
                {
                    int value;
                    int locals_idx = 0;

                    PCMOVE(1);
                    READ_BYTE(locals_idx, current_frame->pc);
                    LOAD(value, int, locals_idx);
                    PUSH(value, int);
                    break;
                }
            case OPC_LLOAD://22
                {
                    int locals_idx = 0;
                    long long value;

                    PCMOVE(1);
                    READ_BYTE(locals_idx, current_frame->pc);
                    LOAD(value, long long, locals_idx);
                    /*NOTE: long use PUSH2*/
                    PUSH2(value, long long);

                    break;
                }
            case OPC_FLOAD://23
                {
                    int locals_idx = 0;
                    float value;

                    PCMOVE(1);
                    READ_BYTE(locals_idx, current_frame->pc);
                    LOAD(value, float, locals_idx);
                    PUSH(value, float);
                    break;
                }
            case OPC_DLOAD://24
                {
                    int locals_idx = 0;
                    double value;

                    PCMOVE(1);
                    READ_BYTE(locals_idx, current_frame->pc);
                    LOAD(value, double, locals_idx);

                    PUSH2(value, double);
                    break;
                }
            case OPC_ALOAD://25
                {
                    Object* objref;
                    int locals_idx = 0;

                    PCMOVE(1);
                    READ_BYTE(locals_idx, current_frame->pc);
                    LOAD(objref, Object*, locals_idx);
                    PUSH(objref, Object*);
                    break;
                }
            case OPC_ILOAD_0://26 NOTE: not test
                {
                    int value;
                    LOAD(value, int, 0);
                    PUSH(value, int);
                    break;
                }
            case OPC_ILOAD_1://27
                {
                    int value;
                    LOAD(value, int, 1);
                    PUSH(value, int);
                    break;
                }
            case OPC_ILOAD_2://28
                {
                    int value;
                    LOAD(value, int , 2);
                    PUSH(value, int);
                    break;
                }
            case OPC_ILOAD_3://29
                {
                    int value;
                    LOAD(value, int, 3);
                    PUSH(value, int);
                    break;
                }
            case OPC_LLOAD_0://30
                {
                    long long value;
                    LOAD(value, long long , 0);
                    PUSH2(value ,long long);
                    break;
                }
            case OPC_LLOAD_1://31
                {
                    long long value;
                    LOAD(value, long long, 1);
                    PUSH2(value, long long);
                    break;
                }
            case OPC_LLOAD_2://32
                {
                    long long value;
                    LOAD(value, long long, 2);
                    PUSH2(value, long long);
                    break;
                }
            case OPC_LLOAD_3://33
                {
                    long long value;
                    LOAD(value, long long ,3);
                    PUSH2(value, long long);
                    break;
                }
            case OPC_FLOAD_0://34
                {
                    float value;
                    LOAD(value, float, 0);
                    PUSH(value, float);
                    break;
                }
            case OPC_FLOAD_1://35
                {
                    float value;
                    LOAD(value, float, 1);
                    PUSH(value, float);
                    break;
                }
            case OPC_FLOAD_2://36
                {
                    float value;
                    LOAD(value, float, 2);
                    PUSH(value, float);
                    break;
                }
            case OPC_FLOAD_3://37
                {
                    float value;
                    LOAD(value, float, 3);
                    PUSH(value, float);
                    break;
                }
            case OPC_DLOAD_0://38
                {
                    double value;
                    LOAD(value, double, 0);
                    PUSH2(value, double);
                    break;
                }
            case OPC_DLOAD_1://39
                {
                    double value;
                    LOAD(value, double, 1);
                    PUSH2(value, double);
                    break;
                }
            case OPC_DLOAD_2://40
                {
                    double value;
                    LOAD(value, double, 2);
                    PUSH2(value, double);
                    break;
                }
            case OPC_DLOAD_3://41
                {
                    double value;
                    LOAD(value, double, 3);
                    PUSH2(value, double);
                    break;
                }
            case OPC_ALOAD_0://42
                {
                    Object* obj;
                    LOAD(obj, Object*, 0);
                    PUSH(obj, Object*);
                    break;
                }
            case OPC_ALOAD_1://43
                {
                    Object* obj;
                    LOAD(obj, Object*, 1);
                    PUSH(obj, Object*);
                    break;
                }
            case OPC_ALOAD_2://44
                {
                    Object* obj;
                    LOAD(obj, Object*, 2);
                    PUSH(obj, Object*);
                    break;
                }
            case OPC_ALOAD_3://45
                {
                    Object* obj;
                    LOAD(obj, Object*, 3);
                    PUSH(obj, Object*);
                    break;
                }
            case OPC_IALOAD://46
                {
                   // printStack();//for test

                    int value, index;
                    Object* obj;
                    POP(index, int);
                    POP(obj, Object*);

                    if (obj == NULL)
                      throwException("NullPointerException");
                    if (!obj->isArray)
                      throwException("NoArray");
                    if (index < 0 || index >= obj->length)
                      throwException("OutofArrayBound");

                    //value = obj->data[index];
                    value = ARRAY_DATA(obj, index, int);

                    PUSH(value, int);

                    break;
                }
            case OPC_LALOAD://47
                {
                    int index;
                    Object* obj;
                    long long value;

                    POP(index, int);
                    POP(obj, Object*);


                    value = ARRAY_DATA(obj, index, long long);
                    PUSH2(value, long long);
                    break;
                }
            case OPC_FALOAD://48
                {
                    float value;
                    int index;
                    Object* obj;

                    POP(index, int);
                    POP(obj, Object*);

                    if (obj == NULL)
                      throwException("NullPointerException");
                    if (!obj->isArray)
                      throwException("NoArray");
                    if (index < 0 || index >= obj->length)
                      throwException("OutofArrayBound");

                    value = ARRAY_DATA(obj, index, float);
                    PUSH(value, float);

                    break;
                }
            case OPC_DALOAD://49
                {
                    double value;
                    int index;
                    Object* obj;

                    POP(index, int);
                    POP(obj, Object*);

                    if (obj == NULL)
                      throwException("NullPointerException");
                    if (!obj->isArray)
                      throwException("obj is not array");
                    if (index < 0 || index >= obj->length)
                      throwException("OutofArrayBound");

                    value = ARRAY_DATA(obj, index, double);
                    PUSH2(value, double);

                    break;
                }
            case OPC_AALOAD://50
                {
                    int index;
                    Object* arrayref;
                    Object* value;

                    POP(index, int);
                    POP(arrayref, Object*);

                    if (arrayref == NULL)
                      throwException("NullPointerException");
                    if (!arrayref->isArray)
                      throwException("obj is not array");
                    if (index < 0 || index >= arrayref->length)
                      throwException("OutofArrayBound");

                    Class* class = arrayref->class;
                    ClassBlock* cb = CLASS_CB(class);
                    if (0 == strcmp(cb->this_classname, "[Ljava/util/Hashtable$HashEntry;"))
                      printf("%s\n", cb->this_classname);

                    //printObjectWrapper(arrayref);

                    value = ARRAY_DATA(arrayref, index, Object*);
                    PUSH(value, Object*);


                    break;
                }
            case OPC_BALOAD://51
                {
                    char value;
                    int index;
                    Object* obj;

                    POP(index, int);
                    POP(obj, Object*);

                    if (obj == NULL)
                      throwException("NullPointerException");
                    if (!obj->isArray)
                      throwException("obj is not array");
                    if (index < 0 || index >= obj->length)
                      throwException("OutofArrayBound");

                    value = ARRAY_DATA(obj, index, char);
                    int _value = (int)value;
                    PUSH(value, int);

                    break;
                }
            case OPC_CALOAD://52
                {
                    int index;
                    Object* obj;
                    char value;

                    POP(index, int);
                    POP(obj, Object*);

                    if (obj == NULL)
                      throwException("NullPointerException");
                    if (!obj->isArray)
                      throwException("obj is not array");
                    if (index < 0 || index >= obj->length)
                      throwException("OutofArrayBound");

                    /*NOTE: The element_size is 2*/
                    value = ARRAY_DATA(obj, index, u2);
                    PUSH(value, char);
                    break;
                }
            case OPC_SALOAD://53
                {
                    int index;
                    Object* obj;
                    short value;

                    POP(index, int);
                    POP(obj, Object*);

                    if (obj == NULL)
                      throwException("NullPointerException");
                    if (!obj->isArray)
                      throwException("obj is not array");
                    if (index < 0 || index >= obj->length)
                      throwException("OutofArrayBound");


                    value = ARRAY_DATA(obj, index, short);
                    int _value = (int)value;
                    PUSH(_value, int);
                    break;
                }
            case OPC_ISTORE://54
                {
                    int value;
                    int locals_idx = 0;
                    PCMOVE(1);
                    READ_BYTE(locals_idx, current_frame->pc);
                    POP(value, int);
                    STORE(value, int, locals_idx);
                    break;
                }
            case OPC_LSTORE://55
                {
                    long long value;
                    int locals_idx = 0;

                    PCMOVE(1);
                    READ_BYTE(locals_idx, current_frame->pc);
                    POP2(value, long long);

                    /*NOTE: The long and double use STORE and LOAD as well*/
                    STORE(value, long long, locals_idx);

                    break;
                }
            case OPC_FSTORE://56
                {
                    float value;
                    int locals_idx = 0;

                    PCMOVE(1);
                    READ_BYTE(locals_idx, current_frame->pc);
                    POP(value, float);
                    STORE(value, float, locals_idx);
                    break;
                }
            case OPC_DSTORE://57
                {
                    double value;
                    int locals_idx = 0;

                    PCMOVE(1);
                    READ_BYTE(locals_idx, current_frame->pc);
                    POP2(value, double);

                    STORE(value, double, locals_idx);
                    break;
                }
            case OPC_ASTORE://58
                {
                    int value = 0;
                    Object* obj;
                    PCMOVE(1);
                    READ_BYTE(value, current_frame->pc);
                    POP(obj, Object*);
                    STORE(obj, Object*, value);

                    break;
                }
            case OPC_ISTORE_0://59
                {
                    int value;
                    POP(value, int);
                    STORE(value, int, 0);

                    break;
                }
            case OPC_ISTORE_1://60
                {
                    int value;
                    POP(value, int);
                    STORE(value, int , 1);
                    break;
                }
            case OPC_ISTORE_2://61
                {
                    int value;
                    POP(value, int);
                    STORE(value, int, 2);
                    break;
                }
            case OPC_ISTORE_3://62
                {
                    int value;
                    POP(value, int);
                    STORE(value, int, 3);
                    break;
                }
            case OPC_LSTORE_0://63
                {
                    long long value;
                    POP2(value, long long);
                    STORE(value, long long, 0);
                    break;
                }
            case OPC_LSTORE_1://64
                {
                    long long value;
                    POP2(value, long long);
                    STORE(value, long long ,1);
                    break;
                }
            case OPC_LSTORE_2://65
                {
                    long long value;
                    POP2(value, long long);
                    STORE(value, long long, 2);
                    break;
                }
            case OPC_LSTORE_3://66
                {
                    long long value;
                    POP2(value, long long);
                    STORE(value, long long, 3);
                    break;
                }
            case OPC_FSTORE_0://67
                {
                    float value;
                    POP(value, float);
                    STORE(value, float, 0);

                    break;
                }
            case OPC_FSTORE_1://68
                {
                    float value;
                    POP(value, float);
                    STORE(value, float, 1);
                    break;
                }
            case OPC_FSTORE_2://69
                {
                    float value;
                    POP(value, float);
                    STORE(value, float, 2);
                    break;
                }
            case OPC_FSTORE_3://70
                {
                    float value;
                    POP(value, float);
                    STORE(value, float, 3);
                    break;
                }
            case OPC_DSTORE_0://71
                {
                    double value;
                    POP2(value, double);
                    STORE(value, double, 0);
                    break;
                }
            case OPC_DSTORE_1://72
                {
                    double value;
                    POP2(value, double);
                    STORE(value, double , 1);
                    break;
                }
            case OPC_DSTORE_2://73
                {
                    double value;
                    POP2(value, double);
                    STORE(value, double, 2);
                    break;
                }
            case OPC_DSTORE_3://74
                {
                    double value;
                    POP2(value, double);
                    STORE(value, double, 3);
                    break;
                }
            case OPC_ASTORE_0://75
                {
                    Object* obj;
                    POP(obj, Object*);
                    STORE(obj, Object*, 0);
                    break;
                }
            case OPC_ASTORE_1://76
                {
                    Object* obj;
                    POP(obj, Object*);
                    STORE(obj, Object*, 1);
                    break;
                }
            case OPC_ASTORE_2://77
                {
                    Object* obj;
                    POP(obj, Object*);
                    STORE(obj, Object*, 2);
                    break;
                }
            case OPC_ASTORE_3://78
                {
                    Object* obj;
                    POP(obj, Object*);
                    STORE(obj, Object*, 3);
                    break;
                }
            case OPC_IASTORE://79
                {
                    Object* obj;
                    int value, index;
                    POP(value, int);
                    POP(index, int);
                    POP(obj, Object*);

                    if (obj == NULL)
                      throwException("NullPointerException");

                    if (!obj->isArray)
                      throwException("NoArray");

                    if (index < 0 || index >= obj->length)
                      throwException("OutofArrayBound");

                    //obj->data[index] = value;
                    ARRAY_DATA(obj, index, int) = value;

                    break;
                }
            case OPC_LASTORE://80
                {
                    Object* obj;
                    long long value;
                    int index;

                    POP2(value, long long);
                    POP(index, int);
                    POP(obj, Object*);

                    ARRAY_DATA(obj, index, long long) = value;
                    break;
                }
            case OPC_FASTORE://81
                {
                    Object* obj;
                    int index;
                    float value;

                    POP(value, float);
                    POP(index, int);
                    POP(obj, Object*);

                    if (obj == NULL)
                      throwException("NullPointerException");

                    if (!obj->isArray)
                      throwException("obj is not array");

                    if (index < 0 || index >= obj->length)
                      throwException("OutofArrayBound");

                    /*NOTE: the type if float*/
                    ARRAY_DATA(obj, index, float) = value;
                    break;
                }
            case OPC_DASTORE://82
                {
                    Object* obj;
                    int index;
                    double value;

                    POP2(value, double);
                    POP(index, int);
                    POP(obj, Object*);

                    if (obj == NULL)
                      throwException("NullPointerException");
                    if (!obj->isArray)
                      throwException("obj is not array");
                    if (index < 0 || index >= obj->length)
                      throwException("OutofArrayBound");

                    ARRAY_DATA(obj, index, double) = value;
                    break;
                }
            case OPC_AASTORE://83
                {
                    Object* arrayref;
                    Object* value;
                    int index;

                    POP(value, Object*);
                    POP(index, int);
                    POP(arrayref, Object*);

                    if (arrayref == NULL)
                      throwException("NullPointerException");
                    if (!arrayref->isArray)
                      throwException("obj is not array");
                    if (index < 0 || index >= arrayref->length )
                      throwException("OutofArrayBount");

                    ARRAY_DATA(arrayref, index, Object*) = value;
                    break;
                }
            case OPC_BASTORE://84
                {
                    int index, value;
                    char _value;
                    Object* obj;

                    POP(value, int);
                    POP(index, int);
                    POP(obj, Object*);
                    _value = (char)value;

                    if (obj == NULL)
                      throwException("NullPointerException");
                    if (!obj->isArray)
                      throwException("obj is not array");
                    if (index < 0 || index >= obj->length)
                      throwException("OutofArrayBound");

                    ARRAY_DATA(obj, index, char) = _value;
                    break;
                }
            case OPC_CASTORE://85
                {
                    Object* objref;
                    int index, value;
                    char _value;

                    POP(value, int);
                    POP(index, int);
                    POP(objref, Object*);
                    _value = (char)value;
                    //*((u2*)(objref->data) + index) = _value;
                    ARRAY_DATA(objref, index, u2) = _value;
                    break;
                }
            case OPC_SASTORE://86
                {
                    Object* obj;
                    int index, value;

                    POP(value, int);
                    POP(index, int);
                    POP(obj, Object*);
                    short _value = (short)value;
                    ARRAY_DATA(obj, index, short) = _value;
                    break;
                }
            case OPC_POP://87
                {
                    int temp;
                    POP(temp, int);
                    break;
                }
            case OPC_POP2://88
                {
                    long long temp;
                    POP2(temp, long long);
                    break;
                }
            case OPC_DUP://89
                {
                    memcpy(current_frame->ostack+1,current_frame->ostack, sizeof(int));
                    current_frame->ostack++;
                    break;
                }
            case OPC_DUP_X1://90
                {
                    int value1, value2;
                    POP(value1, int);
                    POP(value2, int);

                    PUSH(value1, int);
                    PUSH(value2, int);
                    PUSH(value1, int);
                    break;
                }
            case OPC_IADD://96
                {
                    int value1, value2, result;
                    POP(value1, int);
                    POP(value2, int);
                    result = value1 + value2;
                    PUSH(result, int);
                    break;
                }
            case OPC_LADD://97
                {
                    long long value1, value2, result;
                    POP2(value2, long long);
                    POP2(value1, long long);
                    result = value1 + value2;
                    PUSH2(result, long long);
                    break;
                }
            case OPC_FADD://98
                {
                    //TODO
                    float value1, value2, result;
                    POP(value2, float);
                    POP(value1, float);
                    result = value1 + value2;
                    PUSH(result, float);
                    break;
                }
            case OPC_DADD://99
                {
                    double value1, value2, result;
                    POP2(value2, double);
                    POP2(value1, double);
                    result = value1 + value2;
                    PUSH2(result, double);
                    break;
                }
            case OPC_ISUB://100
                {
                    int value1, value2, result;
                    POP(value2, int);
                    POP(value1, int);
                    result = value1 - value2;
                    PUSH(result, int);
                    break;
                }
            case OPC_LSUB://101
                {
                    long long value1, value2, result;
                    POP2(value2, long long);
                    POP2(value1, long long);
                    result = value1 - value2;
                    PUSH2(result, long long);
                    break;
                }
            case OPC_FSUB://102
                {
                    float value1, value2, result;
                    POP(value2, float);
                    POP(value1, float);
                    result = value1 - value2;
                    PUSH(result, float);
                    break;
                }
            case OPC_DSUB://103
                {
                    double value1, value2, result;
                    POP2(value2, double);
                    POP2(value1, double);
                    result = value1 - value2;
                    PUSH2(result, double);
                    break;
                }
            case OPC_IMUL://104
                {
                    int value1, value2, result;
                    POP(value2, int);
                    POP(value1, int);
                    result = value1 * value2;
                    PUSH(result, int);
                    break;
                }
            case OPC_LMUL://105
                {
                    long long value1, value2, result;
                    POP2(value2, long long);
                    POP2(value1, long long);
                    result = value1 * value2;
                    PUSH2(result, long long);
                    break;
                }
            case OPC_FMUL://106
                {
                    float value1, value2, result;
                    POP(value2, float);
                    POP(value1, float);
                    result = value1 * value2;
                    PUSH(result, float);
                    break;
                }
            case OPC_DMUL://107
                {
                    double value1, value2, result;

                    POP2(value2, double);
                    POP2(value1, double);

                    result = value1 * value2;

                    PUSH2(result, double);
                    break;
                }
            case OPC_IDIV://108
                {
                    int value1, value2, result;
                    POP(value2, int);

                    if(0 == value2)
                      throwException("ArithmeticException");

                    POP(value1, int);
                    result = value1/value2;
                    PUSH(result, int);
                    break;
                }
            case OPC_LDIV://109
                {
                    long long value1, value2, result;
                    POP2(value2, long long);

                    if (0 == value2)
                      throwException("ArithmeticException");

                    POP2(value1, long long);
                    result = value1 / value2;
                    PUSH2(result, long long);
                    break;
                }
            case OPC_FDIV://110
                {
                    float value1, value2, result;
                    POP(value2, float);
                    if (0 == value2)
                      throwException("ArithmeticException");

                    POP(value1, float);
                    result = value1/value2;
                    PUSH(result, float);
                    break;
                }
            case OPC_DDIV://111
                {
                    double value1, value2, result;

                    POP2(value2, double);
                    if (0 == value2)
                      throwException("ArithmeticException");
                    POP2(value1, double);
                    result = value1/value2;
                    PUSH2(result, double);
                    break;
                }
            case OPC_IREM://112
                {
                    /*NOTE: The JVM Specification define is
                     *      value1-(value1/value2)*value2
                     *@qcliu 2015/03/15
                     */
                    int value1, value2, result;
                    POP(value2, int);

                    if (0 == value2)
                      throwException("ArithmeticException");

                    POP(value1, int);
                    result = value1 - (value1/value2)*value2;
                    PUSH(result, int);
                    break;
                }
            case OPC_LREM://113
                {
                    long long value1, value2, result;
                    POP2(value2, long long);

                    if (0 == value2)
                      throwException("ArithmeticException");

                    POP2(value1, long long);
                    result = value1 % value2;
                    PUSH2(result, long long);

                    break;
                }
                /*
            case OPC_FREM://114
                {
                    float value1, value2, result;
                    POP(value2, float);
                    if (0 == value2)
                      throwException("ArithmeticException");
                    POP(value1, float);
                    result = value1 -(value2*(value1/value2));
                    PUSH(result, float);
                    break;
                }
            case OPC_DREM://115
                {
                    double value1, value2, result;

                    POP2(value2, double);
                    if (0 == value2)
                      throwException("ArithmeticException");
                    POP2(value1, double);
                    result = value1 -(value2*(value1/value2));
                    PUSH2(result, double);
                    break;
                }
                */
            case OPC_INEG://116
                {
                    int value, result;
                    POP(value, int);
                    result = 0 - value;
                    PUSH(result, int);
                    break;
                }
            case OPC_LNEG://117
                {
                    long long value, result;
                    POP2(value, long long);
                    result = 0 - value;
                    PUSH2(result, long long);
                    break;
                }
            case OPC_FNEG://118
                {
                    float value, result;

                    POP(value, float);
                    result =-value;
                    PUSH(result, float);
                    break;
                }
            case OPC_DNEG://119
                {
                    double value, result;

                    POP2(value, double);
                    result = -value;
                    PUSH2(result, double);

                    break;
                }
            case OPC_ISHL://120
                {
                    int value1, value2, result;

                    POP(value2, int);
                    POP(value1, int);
                    value2 = value2 & 0x0000001f;
                    result = value1 << value2;
                    PUSH(result, int);

                    break;
                }
            case OPC_LSHL://121
                {
                    long long value1, result;
                    int value2;

                    POP(value2, int);
                    POP2(value1, long long);
                    value2 = value2 & 0x0000003f;
                    result = value1 << value2;

                    PUSH2(result, long long);
                    break;
                }
            case OPC_ISHR://122
                {
                    int value1, value2, result;

                    POP(value2, int);
                    POP(value1, int);
                    value2 = value2 & 0x0000001f;
                    result = value1 >> value2;
                    PUSH(result, int);

                    break;
                }
            case OPC_LSHR://123
                {
                    long long value1, result;
                    int value2;

                    POP(value2, int);
                    POP2(value1, long long);
                    value2 = value2 & 0x0000003f;
                    result = value1 >> value2;

                    PUSH2(result, long long);
                    break;
                }
            case OPC_IUSHR://124
                {
                    int value2, result;
                    unsigned int value1;

                    POP(value2, int);
                    POP(value1, unsigned int);
                    value2 = value2 & 0x0000001f;
                    result = value1 >> value2;
                    PUSH(result, int);

                    break;
                }
            case OPC_LUSHR://125
                {
                    long long value1, result;
                    int value2;

                    POP(value2, int);
                    POP2(value1, unsigned long long);
                    value2 = value2 & 0x0000003f;
                    result = value1 >> value2;
                    PUSH2(result, long long);

                    break;
                }
            case OPC_IAND://126
                {
                    int value1, value2;

                    POP(value2, int);
                    POP(value1, int);
                    value1 = value1 & value2;
                    PUSH(value1, int);
                    break;
                }
            case OPC_LAND://127
                {
                    long long value1, value2;
                    POP2(value2, long long);
                    POP2(value1, long long);
                    value1 = value1 & value2;
                    PUSH2(value1, long long);
                    break;
                }
            case OPC_IOR://128
                {
                    int value1, value2;

                    POP(value2, int);
                    POP(value1, int);
                    value1 = value1|value2;
                    PUSH(value1, int);

                    break;
                }
            case OPC_LOR://129
                {
                    long long value1, value2;

                    POP2(value2, long long);
                    POP2(value1, long long);
                    value1 = value1 | value2;
                    PUSH2(value1, long long);

                    break;
                }
            case OPC_IXOR://130
                {
                    int value1, value2;

                    POP(value2, int);
                    POP(value1, int);
                    value1 = value1 ^ value2;
                    PUSH(value1, int);

                    break;
                }
            case OPC_LXOR://131
                {
                    long long value1, value2;

                    POP2(value2, long long);
                    POP2(value1, long long);
                    value1 = value1 ^ value2;
                    PUSH2(value1, long long);

                    break;
                }
            case OPC_IINC://132
                {
                    u4 index;

                    int constt, value;
                    char c;
                    PCMOVE(1);
                    READ_BYTE(index, current_frame->pc);
                    PCMOVE(1);

                    /*NOTE: extend the byte constt to int*/
                    constt = 0;
                    READ_BYTE(c, current_frame->pc);
                    constt = (int)c;

                    LOAD(value, int, index);
                    value += constt;
                    STORE(value, int, index);

                    break;
                }
            case OPC_I2L://133
                {
                    int value;
                    long long result;

                    POP(value, int);
                    result = (long long)value;
                    PUSH2(result, long long);
                    break;
                }
            case OPC_I2F://134
                {
                    int value;
                    float result;

                    POP(value, int);
                    result = (float)value;
                    PUSH(result, float);

                    break;
                }
            case OPC_I2D://135
                {
                    int value;
                    double result;

                    POP(value, int);
                    result = (double)value;
                    PUSH2(result, double);

                    break;
                }
            case OPC_L2I://136
                {
                    long long value;
                    int result;

                    POP2(value, long long);
                    result = (int)value;
                    PUSH(result, int);

                    break;
                }
            case OPC_L2F://137
                {
                    long long value;
                    float result;

                    POP2(value, long long);
                    result = (float)value;
                    PUSH(result, float);

                    break;
                }
            case OPC_L2D://138
                {
                    long long value;
                    double result;

                    POP2(value, long long);
                    result = (double)value;
                    PUSH2(result, double);

                    break;
                }
            case OPC_F2I://139
                {
                    float value;
                    int result;

                    POP(value, float);
                    result = (int)value;
                    PUSH(result, int);

                    break;
                }
            case OPC_F2L://140
                {
                    float value;
                    long long result;

                    POP(value, float);
                    result = (long long)value;
                    PUSH2(result, long long);

                    break;
                }
            case OPC_F2D://141
                {
                    float value;
                    double result;

                    POP(value, float);
                    result = (double)value;
                    PUSH2(result, double);

                    break;
                }
            case OPC_D2I://142
                {
                    double value;
                    int result;

                    POP2(value, double);
                    result = (int)value;
                    PUSH(result, int);
                    break;
                }
            case OPC_D2L://143
                {
                    double value;
                    long long result;

                    POP2(value, double);
                    result = (long long)value;
                    PUSH2(result, long long);

                    break;
                }
            case OPC_D2F://144
                {
                    double value;
                    float result;

                    POP2(value, double);
                    result = (float)value;
                    PUSH(result, float);

                    break;
                }
                /*
                 *operate the byte as int
                 */
            case OPC_I2B://145
                {
                    //int value;
                    //char result;

                   // POP(value, int);
                   // result = (char)value;
                   // PUSH(result, char);
                    break;
                }
            case OPC_I2C://146
                {

                    break;
                }
            case OPC_I2S://147
                {
                    //int value;
                    //short result;

                    //POP(value, int);
                    //result = (short)value;
                    //PUSH(result, short);
                    break;
                }
            case OPC_LCMP://148
                {
                    long long value1, value2;
                    int result;

                    POP2(value2, long long);
                    POP2(value1, long long);
                    if (value1 > value2)
                    {
                        result = 1;
                    }
                    else if (value1 == value2)
                    {
                        result = 0;
                    }
                    else
                    {
                        result = -1;
                    }
                    PUSH(result, int);

                    break;
                }
            case OPC_FCMPG://150
            case OPC_FCMPL://149
                {
                    float value1, value2;
                    int result;

                    POP(value2, float);
                    POP(value1, float);
                    if (value1 > value2)
                    {
                        result = 1;
                    }
                    else if (value1 == value2)
                    {
                        result = 0;
                    }
                    else
                    {
                        result = -1;
                    }
                    PUSH(result, int);
                    break;
                }
            case OPC_IFEQ://153
                {
                    int value;
                    POP(value, int);
                    if (0 == value)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        PCMOVE(offset - 2);

                    }
                    else
                    {
                        PCMOVE(2);
                    }

                    break;
                }
            case OPC_IFNE://154
                {
                    int value;
                    POP(value, int);
                    if (0 != value)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }
                    break;
                }
            case OPC_IFLT://155
                {
                    int value;
                    POP(value, int);
                    if (value < 0)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }
                    break;
                }
            case OPC_IFGE://156
                {
                    int value;
                    POP(value, int);
                    if (value >= 0)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }
                    break;
                }
            case OPC_IFGT://157
                {
                    int value;
                    POP(value, int);
                    if (value > 0)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }
                    break;
                }
            case OPC_IFLE://158
                {
                    int value;
                    POP(value, int);
                    if (value <= 0)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }
                    break;
                }
            case OPC_IF_ICMPEQ://159
                {
                    int value1, value2;

                    POP(value2, int);
                    POP(value1, int);

                    if (value1 == value2)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);

                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }

                    break;
                }
            case OPC_IF_ICMPNE://160
                {
                    int value1, value2;
                    POP(value2, int);
                    POP(value1, int);

                    if (value1 != value2)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);

                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }
                    break;
                }
            case OPC_IF_ICMPLT://161
                {
                    int value1, value2;
                    POP(value2, int);
                    POP(value1, int);
                    if (value1 < value2)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        /*NOTE: the offset */
                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }
                    break;
                }
            case OPC_IF_ICMPGE://162
                {
                    int value1, value2;
                    POP(value2, int);
                    POP(value1, int);
                    if (value1 >= value2)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        /*NOTE: the offset is in terms of OPC_IF_ICMPGE*/

                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }
                    break;
                }
            case OPC_IF_ICMPGT://163
                {
                    int value1, value2;
                    POP(value2, int);
                    POP(value1, int);
                    if (value1 > value2)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }

                    break;
                }
            case OPC_IF_ICMPLE://164
                {
                    int value1, value2;
                    POP(value2, int);
                    POP(value1, int);
                    if (value1 <= value2)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }

                    break;
                }
            case OPC_IF_ACMPEQ://165
                {
                    Object *value1, *value2;
                    POP(value2, Object*);
                    POP(value1, Object*);

                    if (value1 == value2)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }
                    break;
                }
            case OPC_IF_ACMPNE://166
                {
                    Object *value1, *value2;
                    POP(value2, Object*);
                    POP(value1, Object*);
                    if (value1 != value2)
                    {
                        short offset;
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        PCMOVE(offset-2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }
                    break;
                }
            case OPC_GOTO://167
                {
                    short offset;
                    PCMOVE(1);
                    READ_IDX(offset, current_frame->pc);
                    /*
                     * Since before READ_IDX has already pc+1, and
                     * when break switch the there will pc+1 autolly
                     * So, the real offset is offset - 2.
                     */
                    PCMOVE(offset - 2);
                    break;
                }
            case OPC_JSR://168
                {
                    short offset;
                    int address;

                    PCMOVE(1);
                    READ_IDX(offset, current_frame->pc);
                    PCMOVE(1);

                    address = offset;
                    PUSH(address, int);


                    break;
                }
            case OPC_TABLESWITCH://170
                {
                    int def, low, high, index, base, pad, jump;

                    base = pc_offset;//record the location of opc_tableswitch
                    pad = 4-pc_offset%4;//the padding bit(0~3)
                    PCMOVE(pad);
                    READ_INT(def, current_frame->pc);
                    PCMOVE(4);
                    READ_INT(low, current_frame->pc);
                    PCMOVE(4);
                    READ_INT(high, current_frame->pc);
                    PCMOVE(4);

                    POP(index, int);
                    /*default case*/
                    if (index < low || index > high)
                    {
                        int jump = def-(pc_offset - base);
                        PCMOVE(jump - 1);
                        break;
                    }

                    /*other case*/

                    /*
                     * `index-low` determin where is the jump.Compiler can order
                     * the case by lowest to highest automatically.
                     *
                     * `jump` is the offset relative to the current_pc.
                     * But after READ_INT(jump, current_frame->pc), the `jump`
                     * is the offset relative to the OPC_TABLESWITCH which is
                     * represented by `base`. So the pc is alread moved
                     * pc_offset-base relative to base, and now it's still need
                     * move jump-(pc_offset-base).
                     * @qcliu 2015/03/16
                     */
                    PCMOVE((index-low)*4);
                    READ_INT(jump, current_frame->pc);
                    jump = jump - (pc_offset - base);
                    PCMOVE(jump - 1);

                    break;
                }
            case OPC_IRETURN://172
                {
                    //unsigned int* old_ostack = current_frame->prev->ostack;
                    int value;
                    POP(value, int);
                    //*((int*)&current_frame->ret) = value;
                    current_frame->prev->ostack++;
                    *(int*)current_frame->prev->ostack = value;
                    //popFrame();
                    return;
                    break;
                }
            case OPC_FRETURN://174
                {
                    throwException("todo");
                    break;
                }
            case OPC_DRETURN://175
                {
                    double value;
                    POP2(value, double);

                    //*((double*)&current_frame->ret) = value;
                    current_frame->prev->ostack++;
                    *(double*)current_frame->prev->ostack = value;
                    current_frame->prev->ostack++;
                    //popFrame();
                    return;
                    break;
                }
            case OPC_ARETURN://176
                {
                    Object* array_ref;
                    POP(array_ref, Object*);
                    //*((Object**)&current_frame->ret) = array_ref;
                    current_frame->prev->ostack++;
                    *(Object**)current_frame->prev->ostack = array_ref;
                    //popFrame();
                    return;
                    break;
                }
            case OPC_RETURN://177
                {
                    //printf("---------------current  stack top:%d\n", *(int*)current_frame->ostack);
                    //popFrame();
                    return;
                    break;
                }
            case OPC_GETSTATIC://178
                {
                    u2 fieldref_idx;
                    FieldBlock* fb;
                    PCMOVE(1);
                    READ_IDX(fieldref_idx, current_frame->pc);
                    PCMOVE(1);
                    fb = resolveField(current_frame->class, fieldref_idx);

                    if (fb == NULL)
                      throwException("resolveField return NULL");

                    if (0 == strcmp(fb->type, "D"))
                    {
                        double value;
                        value = *(double*)&(fb->static_value);
                        PUSH2(value, double);
                    }
                    else if (0 == strcmp(fb->type, "J"))
                    {
                        long long value;
                        value = *(long long*)&(fb->static_value);
                        PUSH2(value, long long);
                    }
                    else if (0 == strcmp(fb->type, "F"))
                    {
                        float value;
                        value = *(float*)&(fb->static_value);
                        PUSH(value, float);
                    }
                    else
                    {
                        int value;
                        value = fb->static_value;
                        PUSH(value, int);
                    }
                    break;
                }
            case OPC_PUTSTATIC://179
                {
                    u2 fieldref_idx;
                    FieldBlock* fb;
                    PCMOVE(1);
                    READ_IDX(fieldref_idx, current_frame->pc);
                    PCMOVE(1);
                    fb = resolveField(current_frame->class, fieldref_idx);

                    if (0 == strcmp(fb->type, "D"))
                    {
                        double value;
                        POP2(value, double);
                        *(double*)&(fb->static_value) = value;
                    }
                    else if (0 == strcmp(fb->type, "F"))
                    {
                        float value;
                        POP(value, float);
                        *(float*)&(fb->static_value) = value;
                    }
                    else if (0 == strcmp(fb->type, "J"))
                    {
                        long long value;
                        POP2(value, long long);
                        *(long long*)&(fb->static_value) = value;
                    }
                    else
                    {
                        int value;
                        POP(value, int);
                        *(int*)&(fb->static_value) = value;
                    }
                    break;
                }
            case OPC_GETFIELD://180
                {
                    Object* objref;
                    int value, offset;
                    Object* value2;
                    u2 fieldref_idx;

                    PCMOVE(1);
                    READ_IDX(fieldref_idx, current_frame->pc);
                    PCMOVE(1);

                    FieldBlock* fb = resolveField(current_frame->class, fieldref_idx);


                    if (fb == NULL)
                      throwException("resolveField is NULL");
                    //for test
                    if (0 == strcmp(fb->name, "buckets"))
                      printf("TEST: getfield buckets\n");

                    offset = fb->offset;

                    POP(objref, Object*);
                    if (0 == strcmp(fb->type, "J"))
                    {
                        long long value;
                        value = OBJECT_DATA(objref, offset-1, long long);
                        PUSH2(value, long long);
                    }
                    else if (0 == strcmp(fb->type, "D"))
                    {
                        double value;
                        value = OBJECT_DATA(objref, offset-1, double);
                        PUSH2(value, double);
                    }
                    else if (0 == strcmp(fb->type, "F"))
                    {
                        float value;
                        value = OBJECT_DATA(objref, offset-1, float);
                        PUSH(value, float);
                    }
                    else
                    {
                        int value;
                        value = OBJECT_DATA(objref, offset-1, int);
                        PUSH(value, int);
                        //test
                          //printObjectWrapper((Object*)objref);
                    }

                    //value = objref->data[offset - 1];
                    //value = OBJECT_DATA(objref, offset-1, int);

                    //if (0 == strcmp(fb->name, "buckets"))
                      //  printObjectWrapper((Object*)value);

                    //PUSH(value, int);

                    break;
                }
            case OPC_PUTFIELD://181
                {
                    Object* objref;
                    int value, offset;
                    Class* class;
                    u2 fieldref_idx;
                    FieldBlock* fb;


                    PCMOVE(1);
                    READ_IDX(fieldref_idx, current_frame->pc);
                    PCMOVE(1);

                    fb = resolveField(current_frame->class, fieldref_idx);
                    if (fb == NULL)
                      throwException("resolveField is NULL");

                    if (0 == strcmp(fb->name, "buckets"))
                      printf("TEST: putfield buckets\n");

                    offset = fb->offset;//offest always need sub 1;
                    if (0 == strcmp(fb->type, "J"))
                    {
                        long long value;
                        POP2(value, long long);
                        POP(objref, Object*);
                        //*(long long*)&objref->data[offset-1] = value;
                        OBJECT_DATA(objref, offset-1, long long) = value;
                    }
                    else if (0 == strcmp(fb->type, "D"))
                    {
                        double value;
                        POP2(value, double);
                        POP(objref, Object*);
                        //*(double*)&objref->data[offset-1] = value;
                        OBJECT_DATA(objref, offset-1, double) = value;
                    }
                    else if (0 == strcmp(fb->type, "F"))
                    {
                        float value;
                        POP(value, float);
                        POP(objref, Object*);
                        //*(float*)&objref->data[offset-1] = value;
                        OBJECT_DATA(objref, offset-1, float) = value;
                    }
                    else
                    {
                        int value;
                        POP(value, int);
                        POP(objref, Object*);
                        //*(int*)&objref->data[offset-1] = value;
                        OBJECT_DATA(objref, offset-1, int) = value;

                        //for test
                        if (0 == strcmp(fb->name, "buckets"))
                          printObjectWrapper((Object*)value);
                    }


                    /*note: The offset need sub 1, and the Exception is
                     *      throw in resolveField().
                     */
                    //printObject
                    if (dis_testinfo)
                        ;//printObject(objref);

                    break;
                }
            case OPC_INVOKEVIRTUAL://182
                 {
                     u2 methodref_idx, name_type_idx, type_idx;
                     MethodBlock* method;
                     Object* objref;
                     Class* class;
                     u4 cp_info;
                     char* type;
                     int args_count;

                     PCMOVE(1);
                     READ_IDX(methodref_idx, current_frame->pc);
                     PCMOVE(1);
                     /* According to the Methodref_info, get the description of
                      * the method. Then, get the args' count
                      */
                     cp_info = CP_INFO(current_cp, methodref_idx);

                     /*
                      * The methodref_idx maybe resovled.
                      */
                     //===============================================
                     switch (CP_TYPE(current_cp, methodref_idx))
                     {
                         case RESOLVED:
                             {
                                method = (MethodBlock*)cp_info;
                                break;
                             }
                         case CONSTANT_Methodref:
                             {
                                 name_type_idx = cp_info >> 16;
                                 cp_info = CP_INFO(current_cp, name_type_idx);
                                 type_idx = cp_info>>16;
                                 type = CP_UTF8(current_cp, type_idx);

                                 /*
                                  * According to the args_count, find the objectref's location in
                                  * the stack.
                                  * @qcliu 2015/01/30
                                  */
                                 args_count = parseArgs(type);
                                 objref = *(Object**)(current_frame->ostack - args_count);
                                 class = objref->class;
                                 ClassBlock* cb = CLASS_CB(class);

                                 method = (MethodBlock*)resolveMethod(class, methodref_idx);

                                 break;
                             }
                         default:
                             throwException("invokevirtual error!\n");
                     }
                     //===============================================
                     if (!method)
                       throwException("NoSuchMethodError");

                     executeMethod(method, NULL);

                     break;
                 }
                 /*
                  * First, should determine the class is current class or
                  * super class.
                  */
            case OPC_INVOKESPECIAL://183
                 {

                     u2 methodref_idx, class_idx, name_type_idx, name_idx, type_idx;
                     u4 cp_info;
                     Class* class;
                     Class* sym_class;
                     ClassBlock* current_cb;
                     MethodBlock* method;
                     char *sym_classname, *name, *type;

                     current_cb = CLASS_CB(current_frame->class);
                     PCMOVE(1);
                     READ_IDX(methodref_idx, current_frame->pc);
                     PCMOVE(1);
                     cp_info = CP_INFO(current_cp, methodref_idx);

                     /* The methodref_idx maybe resolved*/
                     switch (CP_TYPE(current_cp, methodref_idx))
                     {
                         case RESOLVED:
                             {
                                 method = (MethodBlock*)cp_info;
                                 break;
                             }
                         case CONSTANT_Methodref:
                             {
                                 class_idx = cp_info;

                                 /*get the symbolic class*/
                                 sym_class = (Class*)resolveClass(current_frame->class, class_idx);

                                 name_type_idx = cp_info >> 16;
                                 cp_info = CP_INFO(current_cp, name_type_idx);
                                 name_idx = cp_info;
                                 type_idx = cp_info>>16;

                                 name = CP_UTF8(current_cp, name_idx);
                                 type = CP_UTF8(current_cp, type_idx);

                                 /*
                                  * Determine weather the symbolic class or current class's superclass.
                                  */
                                 if ((strcmp(name, "<init>") !=0) && (current_cb->super == sym_class) &&
                                             (current_cb->access_flags & ACC_SUPER))
                                   class = current_cb->super;
                                 else
                                   class = sym_class;

                                 method = resolveMethod(class, methodref_idx);

                                 break;
                             }
                         default:
                             throwException("invokespecial error!!!");
                     }

                     if (!method)
                       throwException("NoSuchMethodError");

                     executeMethod(method, NULL);

                     break;
                 }
            case OPC_INVOKESTATIC://184
                {
                    u2 methodref_idx;
                    u4 cp_info;
                    u2 class_idx;
                    char* classname;
                    MethodBlock* method;

                    PCMOVE(1);
                    READ_IDX(methodref_idx, current_frame->pc);
                    PCMOVE(1);
                    cp_info = CP_INFO(current_cp, methodref_idx);

                    /*
                     * The methodref_idx maybe resolved.
                     */
                    switch (CP_TYPE(current_cp, methodref_idx))
                    {
                        case RESOLVED:
                            {
                                method = (MethodBlock*)cp_info;
                                break;
                            }
                        case CONSTANT_Methodref:
                            {
                                /*high                    low
                                 *--------------------------
                                 *|name&type |class        |
                                 *--------------------------
                                 */
                                class_idx = cp_info;
                                //classname = CP_UTF8(current_cp, CP_INFO(current_cp, class_idx));

                                /*
                                 * The class is the resovle_method class.It must be inited
                                 * before the method resovle.
                                 * note: in this case, can not use resolveClass. Because we
                                 *       don't know the obj Class's CONSTANT_Class belong to which Class
                                 */
                                //Class* class = (Class*)loadClass(classname);
                                Class* class = (Class*)resolveClass(current_frame->class, class_idx);
                                method = (MethodBlock*)resolveMethod(class, methodref_idx);

                                break;

                            }
                        default:
                            throwException("invokestatic error!!!!");
                    }

                    if (method == NULL)
                      throwException("invokestatic error!!!no such method");

                    executeMethod(method, NULL);

                    break;
                }
            case OPC_INVOKEINTERFACE://185
                {
                     u2 methodref_idx, name_type_idx, type_idx;
                     MethodBlock* method;
                     Object* objref;
                     Class* class;
                     u4 cp_info;
                     char* type;
                     int args_count;

                     PCMOVE(1);
                     READ_IDX(methodref_idx, current_frame->pc);
                     PCMOVE(1);

                     PCMOVE(1);
                     int count = 0;
                     READ_BYTE(count, current_frame->pc);
                     PCMOVE(1);
                     int isZero = 0;
                     READ_BYTE(isZero, current_frame->pc);
                     PCMOVE(1);
                     /* According to the Methodref_info, get the description of
                      * the method. Then, get the args' count
                      */
                     cp_info = CP_INFO(current_cp, methodref_idx);

                     /*
                      * The methodref_idx maybe resovled.
                      */
                     //===============================================
                     switch (CP_TYPE(current_cp, methodref_idx))
                     {
                         case RESOLVED:
                             {
                                 throwException("invokeinterface resolved");
                             }
                             /*NOTE: this is InterfaceMethodref*/
                         case CONSTANT_InterfaceMethodref:
                             {
                                 name_type_idx = cp_info >> 16;
                                 cp_info = CP_INFO(current_cp, name_type_idx);
                                 type_idx = cp_info>>16;
                                 type = CP_UTF8(current_cp, type_idx);

                                 args_count = parseArgs(type);
                                 objref = *(Object**)(current_frame->ostack - args_count);
                                 class = objref->class;
                                 ClassBlock* cb = CLASS_CB(class);

                                 /*NOTE: resolveInterfaceMethod()*/
                                 method = (MethodBlock*)resolveInterfaceMethod(class, methodref_idx);

                                 break;
                             }
                         default:
                             throwException("invokeInterface error!\n");
                     }
                     //===============================================
                     if (!method)
                       throwException("NoSuchMethodError");

                     executeMethod(method, NULL);

                    break;
                }
            case OPC_NEW://187
                {
                    u2 class_idx;
                    Class* class;
                    Object* obj;

                    PCMOVE(1);
                    READ_IDX(class_idx, current_frame->pc);
                    PCMOVE(1);
                    class = resolveClass(current_frame->mb->class, class_idx);
                    obj = allocObject(class);
                    PUSH(obj, Object*);

                    break;
                }
            case OPC_NEWARRAY://188
                {
                    int count, atype;
                    Object* obj;
                    PCMOVE(1);
                    READ_BYTE(atype, current_frame->pc);
                    POP(count, int);

                    obj = (Object*)allocTypeArray(atype, count, NULL);
                    if (obj == NULL)
                      throwException("newarray obj == NULL!");

                    PUSH(obj, Object*);

                    break;
                }
            case OPC_ANEWARRAY://189
                {
                    int count;
                    u2 index;
                    Object* obj;
                    Class* class;

                    index = 0;
                    PCMOVE(1);
                    //READ_IDX(index, current_frame->pc);
                    index=(current_frame->pc[0]<<8)|(current_frame->pc[1]);//test
                    PCMOVE(1);

                    /*need to resolveClass*/
                    char* classname;
                    current_frame->cp->type[index];//test
                    switch (CP_TYPE(current_frame->cp, index))
                    {
                        case RESOLVED:
                            {
                                Class* class = (Class*)CP_INFO(current_frame->cp, index);
                                ClassBlock* cb = CLASS_CB(class);
                                classname = cb->this_classname;
                                break;
                            }
                        case CONSTANT_Class:
                            classname = CP_UTF8(current_frame->cp, CP_INFO(current_frame->cp, index));
                            break;
                        default:
                            throwException("anewarray error. not CONSTANS_CLASS");
                    }

                    /*NOTE: important!!!!!!
                     * As referrence array, need reverse the name.The name
                     * in constants_pool is only the element name.
                     * @qcliu 2015/03/28
                     */
                    int len = strlen(classname);
                    char ac_name[len + 4];

                    if (classname[0] == '[')
                      strcat(strcpy(ac_name, "["), classname);
                    else
                      strcat(strcat(strcpy(ac_name, "[L"), classname),";");


                    POP(count ,int);
                    obj = (Object*)allocTypeArray(T_REFERENCE, count, ac_name);
                    if (obj == NULL)
                      throwException("anewarray obj == NULL!");

                    PUSH(obj, Object*);

                    break;
                }
            case OPC_ARRAYLENGTH://190
                {
                    Object* arrayref;
                    int length;

                    POP(arrayref, Object*);

                    Class* class = arrayref->class;
                    ClassBlock* cb = CLASS_CB(class);

                    if (0 == arrayref)
                      throwException("NullPointerException");

                    if (!arrayref->isArray)
                      throwException("this obj is not a array.");

                    length = arrayref->length;
                    PUSH(length, int);

                    break;
                }
            case OPC_ATHROW://191
                {
                    ClassBlock* cb = CLASS_CB(current_frame->class);
                    printf("current_class:%s, current_method:%s,%s",
                                cb->this_classname, current_frame->mb->name, current_frame->mb->type);

                    //for test
                    printList(head);
                    throwException("athrow test");
                    Object* obj;

                    POP(obj, Object*);
                    //TODO
                    /*do some work*/
                    PUSH(obj, Object*);

                    break;
                }
            case OPC_CHECKCAST://192
                {
                    //TODO
                    PCMOVE(1);
                    PCMOVE(1);
                    break;
                }
            case OPC_INSTANCEOF://193
                {
                    if (testnum == 42)
                      printf("4242424242");
                    printf("instanceof------------------------Test info%d\n", testnum++);
                    //TODO
                    Object* obj;
                    u2 index;
                    int result;

                    PCMOVE(1);
                    READ_IDX(index, current_frame->pc);
                    PCMOVE(1);

                    POP(obj, Object*);

                    if (obj == NULL)
                    {
                      result = 0;
                    }
                    else
                    {
                        Class* class = resolveClass(current_frame->class, index);
                        if (class == NULL)
                          throwException("instanceof error");

                        //result = instanceOf(obj, class);
                        result = isInstanceOf(class, obj->class);
                    }


                    PUSH(result, int);
                    break;
                }
            case OPC_MONITORENTER://194
                {
                    //TODO
                    /*only for test*/
                    Object* obj;
                    POP(obj, Object*);
                    break;
                }
            case OPC_MONITOREXIT://195
                {
                    //TODO
                    Object* obj;
                    POP(obj, Object*);
                    break;
                }
            case OPC_IFNULL://198
                {
                    /*the same as OPC_IF_ICMPGE
                     */
                    Object* obj;
                    short offset;

                    POP(obj ,Object*);

                    if (obj == NULL)
                    {
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }

                    break;
                }
            case OPC_IFNONNULL://199
                {
                    /*the same as OPC_IF_ICMPGE
                     */
                    Object* obj;
                    short offset;

                    POP(obj ,Object*);

                    if (obj != NULL)
                    {
                        PCMOVE(1);
                        READ_IDX(offset, current_frame->pc);
                        PCMOVE(offset - 2);
                    }
                    else
                    {
                        PCMOVE(2);
                    }

                    break;
                }
            default:
                printStack();
                printf("\nwrong opcode!!%s\n", op_code[c]);
                exit(0);
        }
        PCMOVE(1);
        //printStack(); //for test
    }


}
