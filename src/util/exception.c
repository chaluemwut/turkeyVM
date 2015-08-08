#include <stdio.h>
#include <stdlib.h>
#include "testvm.h"
#include "../main/turkey.h"
#include "../lib/hash.h"
#include "../interp/stackmanager.h"

extern JFrame_t current_frame;

void throwException(char* exception) {
    //JFrame_t current_frame = getCurrentFrame();
    printf("\n\e[31m\e[1mException:\e[0m %s\n", exception);
    //print_Stack(current_frame);

    Hash_status(CMap);

    exit(0);
}
