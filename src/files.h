#ifndef FILES_H
#define FILES_H

#include "defs.h"

// Extern Functions
struct FileBuffer *create_file_buffer();
void delete_file_buffer(struct FileBuffer *hb);
struct FileBuffer *load_file(char *fn);
void save_file(struct FileBuffer *hb, char *fn);

#endif
