#include <stdio.h>
#include <stdlib.h>
#include "testvm.h"
#include "../main/turkey.h"
#include "../lib/hash.h"
#include "../interp/stackmanager.h"

extern JFrame_t current_frame;

void throwException(char* exception) {
    JFrame_t current_frame = getCurrentFrame();
    printf("\nException: %s\n", exception);
    print_Stack(current_frame);

    Hash_status(CMap);

    exit(0);
}
