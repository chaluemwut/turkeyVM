#include <stdio.h>
#include <stdlib.h>
#include "javafile.h"
#include "../lib/error.h"
#include "../interp/stackmanager.h"
#include "../lib/string.h"
#include "../util/jstring.h"

#define JF JFrame_t
#define NF NFrame_t
#define O Object_t

extern char* PREFIX;

static const int READ = 1;
static const int WRITE = 2;
static const int APPEND = 4;
static const int EXCL = 8;
static const int SYNC = 16;
static const int DSYNC = 32;

/**
 * Opens the specified file in the specified mode.  This can be done
 * in one of the specified modes:
 * <ul>
 * <li>r - Read Only
 * <li>rw - Read / Write
 * <li>ra - Read / Write - append to end of file
 * <li>rws - Read / Write - synchronous writes of data/metadata
 * <li>rwd - Read / Write - synchronous writes of data.
 *
 * @param path Name of the file to open
 * @param mode Mode to open
 *
 * @returns The resulting file descriptor for the opened file, or -1
 * on failure (exception also signaled).
 *
 * @exception IOException If an error occurs.
 */
 // private native long nativeOpen(String path, int mode)
 //           throws FileNotFoundException;
void nativeOpen(JF retFrame)
{
    NF f = getNativeFrame();

    O path = (O)f->locals[1];
    int mode = (int)f->locals[2];
    char* fpath = String2Char(path);
    FILE* fp = NULL;
    long long r;
    

    //FIXME 
    switch (mode)
    {
        case READ:
            fp = fopen(fpath, "r");
            break;
        case WRITE:
            TODO("unimplements");
            break;
        defualt:
            TODO("unimplements");
            break;
    }
    if (fp == NULL)
      r = -1;
    else
      r = (long long)fp;

    push(retFrame, &r, TYPE_LONG);

    //TODO("todo");

}


#undef JF
#undef O
#undef NF
