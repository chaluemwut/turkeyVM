#include <stdio.h>
#include <stdlib.h>
#include "opcode.h"
#include "../control/control.h"
#include "../lib/assert.h"

/*
 * To control weather print bytecode.
 * @see interp.c
 * @qcliu 2015/01/27
 */
static char *const OPCODE[] = {
    /*{{{ */
    "nop",
    "aconst_null",
    "iconst_m1",
    "iconst_0",
    "iconst_1",
    "iconst_2",
    "iconst_3",
    "iconst_4",
    "iconst_5",
    "lconst_0",
    "lconst_1",
    "fconst_0",
    "fconst_1",
    "fconst_2",
    "dconst_0",
    "dconst_1",
    "bipush",
    "sipush",
    "ldc",
    "ldc_w",
    "ldc2_w",
    "iload",
    "lload",
    "fload",
    "dload",
    "aload",
    "iload_0",
    "iload_1",
    "iload_2",
    "iload_3",
    "lload_0",
    "lload_1",
    "lload_2",
    "lload_3",
    "fload_0",
    "fload_1",
    "fload_2",
    "fload_3",
    "dload_0",
    "dload_1",
    "dload_2",
    "dload_3",
    "aload_0",
    "aload_1",
    "aload_2",
    "aload_3",
    "iaload",
    "laload",
    "faload",
    "daload",
    "aaload",
    "baload",
    "caload",
    "saload",
    "istore",
    "lstore",
    "fstore",
    "dstore",
    "astore",
    "istore_0",
    "istore_1",
    "istore_2",
    "istore_3",
    "lstore_0",
    "lstore_1",
    "lstore_2",
    "lstore_3",
    "fstore_0",
    "fstore_1",
    "fstore_2",
    "fstore_3",
    "dstore_0",
    "dstore_1",
    "dstore_2",
    "dstore_3",
    "astore_0",
    "astore_1",
    "astore_2",
    "astore_3",
    "iastore",
    "lastore",
    "fastore",
    "dastore",
    "aastore",
    "bastore",
    "castore",
    "sastore",
    "pop",
    "pop2",
    "dup",
    "dup_x1",
    "dup_x2",
    "dup2",
    "dup2_x1",
    "dup2_x2",
    "swap",
    "iadd",
    "ladd",
    "fadd",
    "dadd",
    "isub",
    "lsub",
    "fsub",
    "dsub",
    "imul",
    "lmul",
    "fmul",
    "dmul",
    "idiv",
    "ldiv",
    "fdiv",
    "ddiv",
    "irem",
    "lrem",
    "frem",
    "drem",
    "ineg",
    "lneg",
    "fneg",
    "deng",
    "ishl",
    "lshl",
    "ishr",
    "lshr",
    "iushr",
    "lushr",
    "iand",
    "land",
    "ior",
    "lor",
    "ixor",
    "lxor",
    "iinc",
    "i2l",
    "i2f",
    "i2d",
    "l2i",
    "l2f",
    "l2d",
    "f2i",
    "f2l",
    "f2d",
    "d2i",
    "d2l",
    "d2f",
    "i2b",
    "i2c",
    "i2s",
    "lcmp",
    "fcmpl",
    "fcmpg",
    "dcmpl",
    "dcmpg",
    "ifeq",
    "ifne",
    "iflt",
    "ifge",
    "ifgt",
    "ifle",
    "if_icmpeq",
    "if_icmpne",
    "if_icmplt",
    "if_icmpge",
    "if_icmpgt",
    "if_icmple",
    "if_acmpeq",
    "if_acmpne",
    "goto",
    "jsr",
    "ret",
    "tableswitch",
    "lookupswitch",
    "ireturn",
    "lreturn",
    "freturn",
    "dreturn",
    "areturn",
    "return",
    "getstatic",
    "putstatic",
    "getfield",
    "putfield",
    "invokevirtual",
    "invokespecial",
    "invokestatic",
    "invokeinterface",
    "invokedynamic",
    "new",
    "newarray",
    "anewarray",
    "arraylength",
    "athrow",
    "checkcast",
    "instanceof",
    "monitorenter",
    "monitorexit",
    "wide",
    "multianewarray",
    "ifnull",
    "ifnonnull",
    "goto_w",
    "jsr_w",
    "breakpoint",
    /*}}} */
};

char *dumpOpcode(Opcode_e cpcode)
{
    Assert_ASSERT(cpcode >= 0 && cpcode < OPC_NUMBER);

    return OPCODE[cpcode];
}

static int times;
static int statistics[OPC_NUMBER];

void opcodeStatistics(Opcode_e c)
{
    if (0 == Control_opcode)
        return;

    times += 1;
    statistics[c] += 1;
}

static int printSpace(FILE * fd, int i)
{
    int setp = i;
    while (i > 0) {
        fprintf(fd, " ");
        i--;
    }
    return setp;
}

void opcodeStatus()
{
    if (0 == Control_opcode)
        return;

    printf("Generate instruction statistics...\n");
    printf("Find detail in \'instruction.txt\'\n");

    FILE *fd;
    int i;
    int j;

    fd = fopen("instruction.txt", "wr");
    i = 0;
    j = 0;

    j += fprintf(fd, "INSTRUCTION");
    j += printSpace(fd, 30 - j);
    j += fprintf(fd, "TIMES");
    printSpace(fd, 45 - j);
    fprintf(fd, "PERCENT\n");

    while (i < 52) {
        fprintf(fd, "-");
        i++;
    }
    fprintf(fd, "\n");

    i = 0;
    while (i < OPC_NUMBER) {
        j = 0;
        j += fprintf(fd, "%s", OPCODE[i]);
        j += printSpace(fd, 30 - j);
        j += fprintf(fd, "%d", statistics[i]);
        printSpace(fd, 45 - j);
        fprintf(fd, "%.2f%%\n", (float) statistics[i] / (float) times * 100);
        i++;
    }

    i = 0;
    while (i < 52) {
        fprintf(fd, "-");
        i++;
    }
    fprintf(fd, "\n");

    j = 0;
    j += fprintf(fd, "SUM");
    j += printSpace(fd, 30 - j);
    j += fprintf(fd, "%d", times);
    printSpace(fd, 45 - j);
    fprintf(fd, "100%%\n");
}
