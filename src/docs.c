#ifdef __unix__
# error Unix is unsupported
#elif defined _WIN32
# include <io.h>
# include <windows.h>
#endif


#include <stdio.h>

#include "defs.h"
#include "utils.h"



static inline struct DirectoryBuffer *create_directory_buffer() {
    struct DirectoryBuffer *db = malloc(sizeof(struct DirectoryBuffer));
    db->y_len = 0;
    db->buf   = malloc(sizeof(char *) * MAX_BUFSIZE_MED);
    for (int i = 0; i < MAX_BUFSIZE_MED; i++)
        *(db->buf + i) = calloc(MAX_BUFSIZE_SMALL, sizeof(char));

    return db;
}

static inline void delete_directory_buffer(struct DirectoryBuffer *db) {
    for (int i = 0; i < MAX_BUFSIZE_MED; i++)
        free(*(db->buf + i));

    free(db->buf);
    free(db);
}

/*
 * Scans the directory and returns a struct with a list of strings with .c or .h
 * at the end of the filename
 * and a length of the list
 * @FIXME: Allocating monstrous amounts of memory for this and the htmlbuffer
 * @TODO: Error/Edge case checking - Namely if there is no relevant files in "dir"
 * @TODO: Offload the actual directory scanning so I can call it recursively
 */
struct DirectoryBuffer *scan_directory(char *dir) {
    printf("%s\n", dir);

    // Creating struct
    struct DirectoryBuffer *db = create_directory_buffer();

    // Getting full paths
    char srcfiles[MAX_BUFSIZE_MED];
    char hdrfiles[MAX_BUFSIZE_MED];

    str_cpy(dir, srcfiles);
    str_cpy(dir, hdrfiles);

    string_cat("*.c", srcfiles, 3, MAX_BUFSIZE_MED);
    string_cat("*.h", hdrfiles, 3, MAX_BUFSIZE_MED);

    WIN32_FIND_DATAA *file_info = malloc(sizeof(WIN32_FIND_DATAA));

    // Getting first file and the handle for searching
    HANDLE src_srch = FindFirstFileA(srcfiles, file_info);

    if (src_srch == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error in scan_directory: Invalid handle value");
        delete_directory_buffer(db);
        return NULL;
    }

    do {
        str_cpy(dir, *(db->buf + db->y_len));
        string_cat(file_info->cFileName, *(db->buf + db->y_len),
                string_len(file_info->cFileName), MAX_BUFSIZE_SMALL);
        db->y_len++;
    } while (FindNextFileA(src_srch, file_info) != 0);

    HANDLE hdr_srch = FindFirstFileA(hdrfiles, file_info);
    if (hdr_srch == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error in scan_directory: Invalid handle value");
        delete_directory_buffer(db);
        return NULL;
    }

    do {
        str_cpy(dir, *(db->buf + db->y_len));
        string_cat(file_info->cFileName, *(db->buf + db->y_len),
                string_len(file_info->cFileName), MAX_BUFSIZE_SMALL);
        db->y_len++;
    } while (FindNextFileA(hdr_srch, file_info) != 0);

    FindClose(src_srch);
    FindClose(hdr_srch);
    return db;
}
