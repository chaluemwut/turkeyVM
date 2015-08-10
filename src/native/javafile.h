#ifndef JFILE_H
#define JFILE_H

#include "../interp/stackmanager.h"

#define JF JFrame_t

extern void nativeOpen(JF retFrame);

extern void nativeClose(JF retFrame);

extern void nativeReadBuf(JF retFrame);

extern void nativeClose();

#undef JF

#endif
