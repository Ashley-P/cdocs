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
//static inline
struct Identifier *string_to_identifier(const char *str, int len);
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
    char *srcfiles = calloc(MAX_BUFSIZE_MED, sizeof(char));
    char *hdrfiles = calloc(MAX_BUFSIZE_MED, sizeof(char));

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
    free(srcfiles);
    free(hdrfiles);
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

    /**
     * Cull the lines
     * We assume that functions have no leading spaces
     * Our culling assumes that functions have a '(' and ')' and '{'
     */
    int a, b, c;
    int str_len;
    int i;

    for (i = 0; i < fb->y_len_true; i++) {
        a = 0; b = 0; c = 0;

        if (**(fb->buf + i) == ' ' || **(fb->buf + i) == '\n') {
            // No mallocing the free space because it won't be used again
            free(*(fb->buf + i));
            *(fb->buf + i) = NULL;
        } else {
            str_len = string_len(*(fb->buf + i));
            for (int j = 0; j < str_len; j++) {
                if (*(*(fb->buf + i) + j) == '(') a = 1;
                else if (*(*(fb->buf + i) + j) == ')') b = 1;
                else if (*(*(fb->buf + i) + j) == '{') c = 1;
            }

            if (!a || !b || !c) {
                free(*(fb->buf + i));
                *(fb->buf + i) = NULL;
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
    struct Node *func_list = NULL;
    struct FunctionDecon *func;
    struct Identifier *identifier;
    //int param_len;
    for (int i = 0; i < fb->y_len; i++) {
        if (*(fb->buf + i) == NULL) continue;
        func = malloc(sizeof(struct FunctionDecon));
        func->params = NULL;
        // Reusing str_len;
        str_len = seek_to_character(*(fb->buf + i), '(');

        // Getting the identifier for the function itself
        identifier = string_to_identifier(*(fb->buf + i), str_len);
        func->ident = *identifier;
        printf("Name  : %s\tType  : %s\tPointers : %d\tExtra : %d\t", func->ident.name, func->ident.type,
                func->ident.ptr, func->ident.extra);
        printf("\n");

        // Getting the parameters for the functions
        char *param_start = *(fb->buf + i) + str_len + 1;
        int comma_dist;
        int paren_dist;
        printf("%s\n", param_start);

        while (1) {
            comma_dist = seek_to_character(param_start, ',');
            paren_dist = seek_to_character(param_start, ')');
            if (paren_dist <= 1) break;
            else if (comma_dist == -1) {
                identifier = string_to_identifier(param_start, paren_dist);
                printf("Name  : %s\tType  : %s\tPointers : %d\tExtra : %d\n", identifier->name,
                        identifier->type, identifier->ptr, identifier->extra);
                param_start = consume_spaces(param_start + paren_dist + 1);
                printf("%s\n", param_start);
                list_push_back(func->params, create_node(identifier));
            } else {
                identifier = string_to_identifier(param_start, comma_dist);
                printf("Name  : %s\tType  : %s\tPointers : %d\tExtra : %d\n", identifier->name,
                        identifier->type, identifier->ptr, identifier->extra);
                param_start = consume_spaces(param_start + comma_dist + 1);
                printf("%s\n", param_start);
                list_push_back(func->params, create_node(identifier));
            }
        }

        printf("\n\n");
        list_push_back(func_list, create_node(func));
    }

    return func_list;

    free(identifier);
}

/**
 * This function scans the supplied string and returns an "Identifier" struct
 * Takes a length argument so I don't have to break up a string before entering it in
 */
//static inline
struct Identifier *string_to_identifier(const char *str, int len) {
    //printf("%s\n", str);
    struct Identifier *ident = malloc(sizeof(struct Identifier));
    struct FileBuffer *buf = create_file_buffer(MAX_BUFSIZE_TINY);
    ident->ptr   = 0;
    ident->extra = 0;

    // Split up the string
    int a = 0;
    int b = 0;
    buf->y_len = 1;
    char *bufstr = *(buf->buf);
    for (int i = 0; i < len; i++) {
        if (*(str + i) == ' ') {
            buf->y_len++;
            bufstr = *(buf->buf + ++a);
            b = 0;
        } else {
            *(bufstr + b++) = *(str + i);
        }
    }

    // Check each string
    for (int i = 0; i < buf->y_len; i++) {
        if (string_cmp(*(buf->buf + i), "static")) {
            ident->extra |= IDENTIFIER_STATIC;
        } else if (string_cmp(*(buf->buf + i), "const")) {
            ident->extra |= IDENTIFIER_CONST;
        } else if (string_cmp(*(buf->buf + i), "inline")) {
            ident->extra |= IDENTIFIER_INLINE;
        } else if (string_cmp(*(buf->buf + i), "struct")  || string_cmp(*(buf->buf + i), "enum")) {
            // For a struct or enum we automatically copy this and the next thing in the buffer to the typ
            sprintf(ident->type, "%s %s", *(buf->buf + i), *(buf->buf + i + 1));
            i++;
        } else if (buf->y_len - 1 == i) {
            // If we are at the end then it's like the name of the identifier
            sprintf(ident->name, "%s", *(buf->buf + i));
        } else {
            sprintf(ident->type, "%s", *(buf->buf + i));
        }
    }

    // Checking for pointers attached to the variable name
    while (*ident->name == '*') {
        ident->ptr++;
        shift_chars_left(ident->name, string_len(ident->name), 1, 0);
    }

    free(buf);
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
