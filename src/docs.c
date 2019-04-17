#ifdef __unix__
# error Unix is unsupported
#elif defined _WIN32
# include <io.h>
# include <windows.h>
#endif


#include <stdio.h>

#include "docs.h"
#include "files.h"
#include "utils.h"




// Function prototypes
static inline struct Identifier *string_to_identifier(const char *str, int len);
static inline int seek_to_character(const char *str, char ch);



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
// Return a linked list of all the functions in 
struct Node *scan_file_functions(char *fp) {
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
    }

    /**
     * Deconstructing what's left
     * Since things can be typedef'd we can only shave off certain things
     * Namely we can search explicitly for "static" "inline" "const" and "struct"
     * Searching for struct let's us take the next word we find and correctly assume that
     * it's the identifier for the struct
     */
    struct Node *head;
    struct FunctionDecon *func;
    //int param_len;
    for (int i = 0; i < fb->y_len; i++) {
        printf("%s", *(fb->buf + i));
        fflush(stdout);
        func = malloc(sizeof(struct FunctionDecon));
        int str_len = seek_to_character(*(fb->buf + i), '(');
        string_to_identifier(*(fb->buf + i), str_len + 1);

    }
    //return fb;
}

/**
 * This function scans the supplied string and returns an "Identifier" struct
 * Takes a length argument so I don't have to break up a string before entering it in
 */
static inline struct Identifier *string_to_identifier(const char *str, int len) {
    struct Identifier *ident = malloc(sizeof(struct Identifier));
    int word_start = 0;
    int word_len;
    while ((word_len = seek_to_character(str, ' ')) != -1) {
        if (word_start + word_len > len) {
            if (word_start < len) {
                str_cpy2((str + word_start), ident->name, word_start - len);
            } else {
                break;
            }
        }
        else if (string_cmp2((str + word_start), "static", word_len)) {
            ident->extra |= 0x4;
        } else if (string_cmp2((str + word_start), "const", word_len)) {
            ident->extra |= 0x2;
        } else if (string_cmp2((str + word_start), "inline", word_len)) {
            ident->extra |= 0x1;
        } else if (string_cmp2((str + word_start), "struct", word_len)) {
            // If the word is struct then we automatically read the next word which is the type
            int tmp = seek_to_character((str + word_start + word_len), ' ');
            str_cpy2((str + word_start), ident->type, tmp);
            word_start = tmp;
            continue;
        } else {
            // Otherwise it's probably a type that we should copy
            str_cpy2((str + word_start), ident->type, word_len);
        }

        word_start += word_len;
    }

    printf("Name  : %s\nType  : %s\nExtra : %d\n", ident->name, ident->type, ident->extra);


    return ident;
}

/**
 * This functions seeks to a provided character and returns the length -1 on failure
 * TODO: Could do with an upgrade to skip occurences of ch up to a user passed number
 */
static inline int seek_to_character(const char *str, char ch) {
    int i = 0;
    while (*(str + i) != ch) {
        if (*(str + i) == '\0') return -1;
        else i++;
    }

    return i;
}
