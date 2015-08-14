#include <stdio.h>
#include <stdlib.h>
#include "javafile.h"
#include "../util/jstring.h"
#include "../interp/stackmanager.h"
#include "../classloader/class.h"
#include "../classloader/resolve.h"
#include "../util/exception.h"
#include "../interp/execute.h"
#include "../lib/error.h"
#include "../lib/string.h"
#include "../lib/mem.h"
#include "../lib/assert.h"

#define JF JFrame_t
#define NF NFrame_t
#define O Object_t
#define C Class_t
#define FD FileDescriptor_t

extern char* PREFIX;

static const int READ = 1;
static const int WRITE = 2;
static const int APPEND = 4;
static const int EXCL = 8;
static const int SYNC = 16;
static const int DSYNC = 32;



typedef struct FD *FD;

struct FD
{
    FILE* fd;
    int reachEOF;
};

//static int reachEOF = 0;


static FD initNativeFile(FILE* fd, int e)
{
    FD fp;
    Mem_new(fp);

    fp->fd = fd;
    fp->reachEOF = e;

    return fp;
}
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
            //printf("mode:%d\n", mode);
            //TODO("unimplements");
            fp = fopen(fpath, "wr");
            break;
        defualt:
            printf("mode:%d\n", mode);
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
    //printf("%lld\n", r);

    FD fdp = initNativeFile(fp, 0);

    r = (long long)fdp;

    push(retFrame, &r, TYPE_LONG);

}

/**
 * Closes this specified file descriptor
 *
 * @parm fd The natice file descriptor to close
 * @return The return code of the natice close command.
 * @exception IOException If an error occurs
 */
//privete native long nativeClose(long fd) throws IOException;
void nativeClose(JF retFrame)
{
    NF n = getNativeFrame();
    O this;
    long long fd;

    LOAD(n, this, O, 0);
    LOAD(n, fd, long long, 1);

    FD fdp = (FD)fd;
    int err = fclose(fdp->fd);
    if (err)
      ERROR("close file error");

    long long r = fd;
    push(retFrame, &r, TYPE_LONG);
}


/**
 * Writes a byte buffer to the file
 *
 * @param fd The native file descriptor to write to
 * @param buf The byte buffer to write from
 * @param int The offset into the buffer to start writing from
 * @param len The number of bytes to write.
 *
 * @return The return code of the native write command
 *
 * @exception IOException If an error occurs
 */
//private native long nativeWriteBuf(long fd, byte[] buf, int offset, int len) 
//throws IOException;
/*FileDescriptor*/
void nativeWriteBuf(JF retFrame)
{
    //JF current_frame = getCurrentFrame();
    NF n = getNativeFrame();

    O this;
    long long fd;
    O buf;
    int _offset;
    int len;
    FD fdp;


    LOAD(n, this, O, 0);
    LOAD(n, fd, long long, 1);
    LOAD(n, buf, O, 3);
    LOAD(n, _offset, int, 4);
    LOAD(n, len, int, 5);

    Assert_ASSERT(fd);
    if ((FILE*)fd == stdout)
       fdp = initNativeFile(stdout, 0);
    else
       fdp = (FD)fd;
    Assert_ASSERT(buf->type == TYPE_ARRAY);
    int r = fwrite(ARRAY_IDX(buf, _offset, char), sizeof(char), len, fdp->fd);

    //int r = fwrite(ARRAY_IDX(buf, _offset, char), sizeof(char), len, stdout);
    //char* p = (char*)buf->data;
    //int i;
    //for (i=_offset; i<len; i++)
    //{
    //    printf("%c", *p);
    //    p++;
    //}

    long long rr = (long long)r;
    push(retFrame, &rr, TYPE_LONG);
    //current_frame->ostack++;
    //*(long long*)current_frame->ostack = 1;
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
    FD fdp = (FD)fd;
    if (fdp->reachEOF == 1)
    {
      int rr = -1;
      push(retFrame, &rr, TYPE_INT);
      return;
    }

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
    int r = fread(ARRAY_IDX(buf, offset, char), sizeof(char), len, fdp->fd);
    /*
    fprintf(stdout, "this:%p\n", this);
    fprintf(stdout, "fd:%lld\n", (long long)fdp->fd);
    fprintf(stdout, "buf:%p\n", buf);
    fprintf(stdout, "offset:%d\n", offset);
    fprintf(stdout, "len:%d\n", len);
    fprintf(stdout, "r:%d\n", r);
    */
    if (r < len)
    {
        //WARNING("r<len");
        //fprintf(stdout, "EOF:%x\n", EOF);
        ARRAY_DATA(buf, r+offset, char) = EOF;
        //char c = ARRAY_DATA(buf, r+offset, char);
        //int i = c;
        //printf("%d\n", i);

    }


    push(retFrame, &r, TYPE_INT);

    if (r < len)
      fdp->reachEOF = 1;

}

//java/io/FileDescriptor
void nativeValid(JF retFrame)
{
    //JF current_frame = getCurrentFrame();
    NF n = getNativeFrame();

    int ret = TRUE;
    long long nativeFd;
    LOAD(n, nativeFd, long long, 1);
    if (nativeFd >= 0)
        ret = TRUE;
    else
        ret = FALSE;

    push(retFrame, &ret, TYPE_INT);

}


void nativeInit(JF retFrame)
{
    C c = findClass("java/io/FileDescriptor");
    FieldBlock* fb = (FieldBlock*)findField(c, "out", "Ljava/io/FileDescriptor;");
    FieldBlock* fb_err = (FieldBlock*)findField(c, "err", "Ljava/io/FileDescriptor;");
    O err = (O)fb_err->static_value;
    O out = (O)fb->static_value;
    Assert_ASSERT(err);
    Assert_ASSERT(out);

    MethodBlock* mb = (MethodBlock*)findMethod(out->class, "<init>", "(J)V");
    if (mb == NULL)
        throwException("no such method!");
    /*hack*/
    executeMethodArgs(out->class, mb, out, stdout);

    MethodBlock* mb_err = (MethodBlock*)findMethod(err->class, "<init>", "(J)V");
    if (mb_err == NULL)
        throwException("no such method!");
    executeMethodArgs(err->class, mb_err, err, stdout);
}


#undef JF
#undef O
#undef NF
#undef C
