#ifndef MEM_H
#define MEM_H


#define Mem_new(p)                  \
    do {    \
        (p) = malloc(sizeof(*(p))); \
    }while(0)


#endif
