#ifndef DOCS_H
#define DOCS_H

#include "defs.h"
#include "ll.h"

// Extern Functions
struct DirectoryBuffer *scan_directory(char *dir);
struct Node *scan_file_functions(char *fp);
struct Node *scan_file_structs(char *fp);
struct Node *scan_file_enums(char *fp);

#endif
