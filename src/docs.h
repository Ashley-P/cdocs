#ifndef DOCS_H
#define DOCS_H

#include "defs.h"
#include "ll.h"

struct Identifier {
    // Name of the function/struct/etc
    char name[MAX_BUFSIZE_MED];
    char type[MAX_BUFSIZE_MED];
    int ptr;
    /**
     * MSB TO LSB
     * is static?
     * is const?
     * is inline?
     */
    char extra:3;
};

// Enum pararms are slightly different from identifiers
struct EnumMember {
    char name[MAX_BUFSIZE_MED];
    int num;
};

// Deconstructed function
struct FunctionDecon {
    struct Identifier ident;

    // Function parameters - Linked list of functions params
    struct List params;

    // source code - NOT YET IMPLEMENTED
    struct FileBuffer;
};

// Deconstructed struct
struct StructDecon {
    struct Identifier ident;

    // Struct parameters - Linked list of struct params
    struct List members;
};

// Deconstructed enum
struct EnumDecon {
    struct Identifier ident;

    // Enum parameters - Linked list of enum params
    struct List enums;
};

struct DefineDecon {
};

// Extern Functions
struct DirectoryBuffer *scan_directory(char *dir);
void scan_files(struct DirectoryBuffer *db, struct List *functions, struct List *structs,
        struct List *enums, struct List *defines);

#endif
