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
    // Creating struct
    struct DirectoryBuffer *db = create_directory_buffer();

    // Getting full paths
    char srcfiles[MAX_BUFSIZE_MED];
    char hdrfiles[MAX_BUFSIZE_MED];

    sprintf(srcfiles, "%s*.c", dir);
    sprintf(hdrfiles, "%s*.h", dir);

#if _WIN32
    WIN32_FIND_DATAA *file_info = malloc(sizeof(WIN32_FIND_DATAA));

    // Getting first file and the handle for searching
    HANDLE src_srch = FindFirstFileA(srcfiles, file_info);
    int files_found = 0;
    printf("Files found %d!\r", files_found);

    if (src_srch == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error in scan_directory: Invalid handle value");
        delete_directory_buffer(db);
        return NULL;
    }

    do {
        sprintf(*(db->buf + db->y_len), "%s%s", dir, file_info->cFileName);
        db->y_len++;
        files_found++;
        printf("Files found %d!\r", files_found);
    } while (FindNextFileA(src_srch, file_info) != 0);

    HANDLE hdr_srch = FindFirstFileA(hdrfiles, file_info);
    if (hdr_srch == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error in scan_directory: Invalid handle value");
        delete_directory_buffer(db);
        return NULL;
    }

    do {
        sprintf(*(db->buf + db->y_len), "%s%s", dir, file_info->cFileName);
        db->y_len++;
        files_found++;
        printf("Files found %d!\r", files_found);
    } while (FindNextFileA(hdr_srch, file_info) != 0);

    printf("\n");
    FindClose(src_srch);
    FindClose(hdr_srch);
#endif

    return db;
}

/**
 * Scans the file and returns each function definition
 * @NOTE:
 * 0. Function prototypes are skipped
 * 1. We assume that each function starts with no indent
 */
void scan_file_functions(char *fp) {
    // We assume that fp is correct
    FILE *f = fopen(fp, "r");
    




    fclose(f);
}
