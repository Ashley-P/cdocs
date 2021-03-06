#ifndef DEFS_H
#define DEFS_H


#define MAX_BUFSIZE_TINY  16
#define MAX_BUFSIZE_MINI  64
#define MAX_BUFSIZE_SMALL 256
#define MAX_BUFSIZE_MED   1024 
#define MAX_BUFSIZE_LARGE 2048
#define MAX_BUFSIZE_SUPER 100000

struct FileBuffer {
    char **buf;
    unsigned short x_len;
    unsigned short y_len;
    unsigned short y_len_true; // The malloc'd sized of buf
};

struct DirectoryBuffer {
    char **buf;
    unsigned short y_len;
    // x_len is always MAX_BUFSIZE_SMALL
    // y_len_true is always MAX_BUFSIZE_MED
};

#define IDENTIFIER_STATIC 0x04
#define IDENTIFIER_CONST  0x02
#define IDENTIFIER_INLINE 0x01


#endif
