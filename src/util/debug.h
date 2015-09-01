#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG(format, ...) printf("FILE: "__FILE__", LINE: %d: "format"\n",__LINE__, ##__VA_ARGS__);

#endif
