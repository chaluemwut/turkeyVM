#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>

#define BUG(s)                      \
    do {                            \
        fprintf(stderr,             \
                    "%s> %d, %s\n", \
                    s,              \
                    __LINE__,       \
                    __FILE__        \
               );                   \
        fflush(stderr);             \
        exit(1);                    \
    }while (0)




#endif
