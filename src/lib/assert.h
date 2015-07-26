#ifndef ASSERT_H
#define ASSERT_H


#include <stdio.h>
#include <stdlib.h>


#define Assert_ASSERT(e)        \
    do {                        \
        if (e)                  \
        ;                       \
        else{                   \
            fprintf(stderr,     \
                        "\nassertion failed:%s:%d> %s\n",   \
                        __FILE__, __LINE__, #e);                    \
            exit(1);                                                \
        }                       \
    }while(0)



#endif
