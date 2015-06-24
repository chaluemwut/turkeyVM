#include <stdio.h>
#include <stdlib.h>
#include "testvm.h"

extern Frame* current_frame;

void throwException(char* exception)
{
    printf("\nException: %s\n", exception);
    ClassBlock* cb = CLASS_CB(current_frame->class);
    printf("current_class:%s, current_method:%s%s\n", cb->this_classname,
                current_frame->mb->name,current_frame->mb->type);
    printStack();
    exit(0);
}
