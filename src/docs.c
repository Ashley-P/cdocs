#ifdef __unix__
# error Unix is unsupported
#elif defined _WIN32
# include <io.h>
# include <windows.h>
#endif


#include <stdio.h>

#include "defs.h"
#include "files.h"
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
 * @FIXME: Allocating monstrous amounts of memory for this and the FileBuffer
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
 * 2. Functions have the opening brace on the same line
 */
struct FileBuffer *scan_file_functions(char *fp) {
    // We assume that fp is correct
    struct FileBuffer *fb = load_file(fp);
    int a, b, c;

    /**
     * Cull the lines
     * We assume that functions have no leading spaces
     * Our culling assumes that functions have a '(' and ')' and '{'
     */
    for (int i = 0; i < fb->y_len; i++) {
        a = 0; b = 0; c = 0;

        if (**(fb->buf + i) == ' ' || **(fb->buf + i) == '\n') {
            // No mallocing the free space because it won't be used again
            shift_pointers_left((void **) fb->buf, fb->y_len_true, 1, i + 1);
            fb->y_len--;
            i--;
        } else {
            for (int j = 0; j < string_len(*(fb->buf + i)); j++) {
                if (*(*(fb->buf + i) + j) == '(') a = 1;
                else if (*(*(fb->buf + i) + j) == ')') b = 1;
                else if (*(*(fb->buf + i) + j) == '{') c = 1;
            }

            if (!a || !b || !c) {
                shift_pointers_left((void **) fb->buf, fb->y_len_true, 1, i + 1);
                fb->y_len--;
                i--;
            }
        }
        // Print out the line we are keeping - no real reason to do this
        //printf("%s", *(fb->buf + i));
        //fflush(stdout);
    }
    printf("\n");
    return fb;
}
