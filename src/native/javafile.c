#include <stdio.h>
#include <stdlib.h>
#include "javafile.h"
#include "../lib/error.h"
#include "../interp/stackmanager.h"
#include "../lib/string.h"
#include "../lib/mem.h"
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
 // private native long nativeOpen(String path, int mode) throws FileNotFoundException;
 // Class: java/io/FileDescriptor 
void nativeOpen(JF retFrame)
{
    NF f = getNativeFrame();

    O path = (O)f->locals[1];
    int mode = (int)f->locals[2];
    char* fpath = Jstring2Char(path);
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
    {
      r = -1;
      WARNING("file not exsit");
    }
    else
      r = (long long)fp;
    printf("%lld\n", r);

    push(retFrame, &r, TYPE_LONG);

}





/**
 * Reads a buffer of  bytes from the file
 *
 * @param fd The native file descriptor to read from
 * @param buf The buffer to read bytes into
 * @param offset The offset into the buffer to start storing bytes
 * @param len The number of bytes to read.
 *
 * @return The number of bytes read, or -1 if end of file.
 *
 * @exception IOException If an error occurs
 */
//
//private native int nativeReadBuf(long fd, byte[] buf, int offset, int len) throws IOException;
//Class: java/io/FileDescriptor
void nativeReadBuf(JF retFrame)
{
    NF n = getNativeFrame();
    O this;// = (O)n->locals[0];
    O buf;
    long long fd;
    int offset;
    int len;

    LOAD(n, this, O, 0);
    LOAD(n, fd, long long, 1);
    LOAD(n, buf, O, 3);
    LOAD(n, offset, int, 4);
    LOAD(n, len, int, 5);

    //printf("%lld\n", fd);
    FILE* fp = (FILE*)fd;

    /* for test
    fseek(fp, 0L, SEEK_END);
    long flen = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    char* buff;

    Mem_newSize(buff, flen+1);
    fread(buff, sizeof(char*), flen, fp);
    buff[flen] = '\0';
    printf("%s\n", buff);
    */
    int r = fread(ARRAY_IDX(buf, offset, char), sizeof(char), len, fp);
    //printf("%s\n", buf->data);
    //printf("%lld\n", r);
    push(retFrame, &r, TYPE_INT);

    //TODO("nativeReadBuf");
}



#undef JF
#undef O
#undef NF
