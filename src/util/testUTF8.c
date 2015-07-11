#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE 1
#define HASHTABSZE 1<<10
#define HASH(ptr) utf8Hash(ptr)
#define COMPARE(ptr1, ptr2, hash1, hash2) (ptr1 == ptr2) || \
                                                 ((hash1 == hash2) && utf8Comp(ptr1, ptr2))
#define PREPARE(ptr) ptr
#define SCAVENGE(ptr) FALSE
#define FOUND(ptr)

#define GET_UTF8_CHAR(ptr, c)                         \
{                                                     \
    int x = *ptr++;                                   \
    if(x & 0x80) {                                    \
        int y = *ptr++;                               \
        if(x & 0x20) {                                \
            int z = *ptr++;                           \
            c = ((x&0xf)<<12)+((y&0x3f)<<6)+(z&0x3f); \
        } else                                        \
        c = ((x&0x1f)<<6)+(y&0x3f);               \
    } else                                            \
    c = x;                                        \
}

int utf8Len(unsigned char *utf8) {
    int count;

    for(count = 0; *utf8; count++) {
        int x = *utf8;
        utf8 += (x & 0x80) ? ((x & 0x20) ?  3 : 2) : 1;
    }

    return count;
}

void convertUtf8(unsigned char *utf8, short *buff) {
    while(*utf8)
      GET_UTF8_CHAR(utf8, *buff++);
}

int utf8Hash(unsigned char *utf8) {
    int hash = 0;

    while(*utf8) {
        short c;
        GET_UTF8_CHAR(utf8, c);
        hash = hash * 37 + c;
    }

    return hash;
}

int utf8Comp(unsigned char *ptr, unsigned char *ptr2) {
    while(*ptr && *ptr2) {
        short c, c2;

        GET_UTF8_CHAR(ptr, c);
        GET_UTF8_CHAR(ptr2, c2);
        if(c != c2)
          return FALSE;
    }
    if(*ptr || *ptr2)
      return FALSE;

    return TRUE;
}


unsigned char *slash2dots(unsigned char *utf8) {
    int len = utf8Len(utf8);
    unsigned char *conv = (unsigned char*)malloc(len+1);
    int i;

    for(i = 0; i <= len; i++)
      if(utf8[i] == '/')
        conv[i] = '.';
      else
        conv[i] = utf8[i];

    return conv;
}

/* Functions used by JNI */

int utf8CharLen(short *unicode, int len) {
    int count = 1;

    for(; len > 0; len--) {
        short c = *unicode++;
        count += c > 0xf ? (c > 0x7ff ? 3 : 2) : 1;
    }

    return count;
}

char *unicode2Utf8(short *unicode, int len) {
    char *utf8 = (char*)malloc(utf8CharLen(unicode, len));
    char *ptr = utf8;

    for(; len > 0; len--) {
        short c = *unicode++;
        if((c == 0) || (c > 0x7f)) {
            if(c > 0x7ff) {
                *ptr++ = (c >> 12) | 0xe0;
                *ptr++ = ((c >> 6) & 0x3f) | 0x80;
            } else
              *ptr++ = (c >> 6) | 0xc0;
            *ptr++ = (c&0x3f) | 0x80;
        } else
          *ptr++ = c;
    }

    *ptr = '\0';
    return utf8;
}
/*
void main(int argc, char** argv)
{
    char* utf8 ="qcliu";
    int len = utf8Len(utf8);
    short* buf = (short*)malloc(len* sizeof(short));
    short* p = buf;

    convertUtf8(utf8, buf);
    while (len > 0)
    {
        char c = (char)(*p);
        printf("%c", c);
        p++;
        len--;
    }


    printf("%d\n", len);




}
*/
