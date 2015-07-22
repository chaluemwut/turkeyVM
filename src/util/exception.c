#include <stdio.h>
#include <stdlib.h>
#include "testvm.h"
#include "../interp/stackmanager.h"

extern JFrame_t current_frame;

void throwException(char* exception) {
    JFrame_t current_frame = getCurrentFrame();
    printf("\nException: %s\n", exception);
    ClassBlock* cb = CLASS_CB(current_frame->class);
    printf("current_class:%s, current_method:%s%s\n", cb->this_classname,
                current_frame->mb->name,current_frame->mb->type);
    printStack();
    exit(0);
}
