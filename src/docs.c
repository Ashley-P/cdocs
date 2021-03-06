#ifdef __unix__
# error Unix is unsupported
#elif defined _WIN32
# include <io.h>
# include <windows.h>
#endif


#include <stdio.h>

#include "docs.h"
#include "files.h"
#include "ll.h"
#include "utils.h"

enum LineType {
    LT_INVALID,
    LT_FUNCTION,
    LT_STRUCT,
    LT_ENUM,
    LT_DEFINE,
    LT_TYPEDEF,
};



// Function prototypes
//enum LineType scan_line(char *str);
static inline struct Identifier *string_to_identifier(const char *str);
static inline struct StructDecon *parse_struct(struct FileBuffer *fb, unsigned int line);
static inline struct EnumDecon *parse_enum(struct FileBuffer *fb, unsigned int line);
static inline struct FunctionDecon *parse_function(struct FileBuffer *fb, unsigned int line);
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

    if (src_srch == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error in scan_directory: Invalid handle value");
        delete_directory_buffer(db);
        return NULL;
    }

    do {
        sprintf(*(db->buf + db->y_len), "%s%s", dir, file_info->cFileName);
        db->y_len++;
        printf("%d files found!\r", ++files_found);
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
        printf("%d files found!\r", ++files_found);
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
 * Scans a line and returns the type of that line
 */
enum LineType scan_line(char *str) {
        int j;

        if (*str == '\n' || *str == ' ' || *str == '/' || *str == '*' || *str == '}') {
            return LT_INVALID;
        } else if (*str == '#') {                                  // Defines
            char *line = consume_spaces(str + 1);
            if (string_cmp2(line, "define", 6)) {
                return LT_DEFINE;
            } else {
                return LT_INVALID;
            }

        } else if (string_cmp2(str, "typedef", 7)) {              // Typedefs
            return LT_TYPEDEF;

        } else if (string_cmp2(str, "struct", 6)) {               // Structs  
            // Make sure it is not a function by checking for some parentheses
            for (j = 0; j < string_len(str); j++) {
                if (*(str + j) == '(')
                    return LT_FUNCTION;
            }
            return LT_STRUCT;

        } else if (string_cmp2(str, "enum", 4)) {                // enums  
            // Make sure it is not a function by checking for some parentheses
            for (j = 0; j < string_len(str); j++) {
                if (*(str + j) == '(')
                    return LT_FUNCTION;
            }
            return LT_ENUM; 

        } else {
            //return LT_INVALID;
            return LT_FUNCTION;
        }

}

/**
 * The new and improved scanning function
 * This scans an individual file and fills out the linked lists
 * Basic assumptions:
 * 0. Anything that we want is unindented
 * 1. Identifiers are declared all on one line
 * 2. Parameters can be on multiple lines
 * 3. Defines are grouped based on empty lines
 * 4. Define lines start with '#' but there can be spaces after that
 * 5. We aren't collecting function prototypes
 * 6. The ends of functions are a single '}' on it's own line unindented
 * 7. Defines can emulate Variables and Functions
 * 8. Structs are not defined inside other structs
 */
void scan_file(char *fp, struct List *functions, struct List *structs, struct List *enums, struct List *defines,
        unsigned int *functions_found, unsigned int *structs_found, unsigned int *enums_found,
        unsigned int *defines_found) {

    struct FileBuffer *file = load_file(fp);

    int i;
    void *tmp; // Just to hold the structs that come from the "parse_*" functions

    for (i = 0; i < file->y_len; i++) {
        switch (scan_line(*(file->buf + i))) {
            case LT_INVALID:
                //continue;
                break;
            case LT_FUNCTION:
                tmp = parse_function(file, i);
                if (tmp) {
                    (*functions_found)++;
                    list_push_back(functions, create_node(tmp));
                }
                break;
            case LT_STRUCT:
                tmp = parse_struct(file, i);
                if (tmp) {
                    (*structs_found)++;
                    list_push_back(structs, create_node(tmp));
                }
                break;
            case LT_ENUM:
                tmp = parse_enum(file, i);
                if (tmp) {
                    (*enums_found)++;
                    list_push_back(enums, create_node(tmp));
                }
                break;
            case LT_DEFINE:
                break;
            default:
                break;
        }
    }
}

/**
 * Scans a DirectoryBuffer and calls scan_file
 */
void scan_files(struct DirectoryBuffer *db, struct List *functions, struct List *structs,
        struct List *enums, struct List *defines) {
    unsigned int *functions_found = malloc(sizeof(unsigned int));
    unsigned int *structs_found   = malloc(sizeof(unsigned int));
    unsigned int *enums_found     = malloc(sizeof(unsigned int));
    unsigned int *defines_found   = malloc(sizeof(unsigned int));
    *functions_found = 0;
    *structs_found   = 0;
    *enums_found     = 0;
    *defines_found   = 0;

    for (int i = 0; i < db->y_len; i++) {
        scan_file(*(db->buf + i), functions, structs, enums, defines,
                functions_found, structs_found, enums_found, defines_found);
    }

    printf("%d functions found!\n", *functions_found);
    printf("%d structs found!\n", *structs_found);
    printf("%d enums found!\n", *enums_found);
    printf("%d defines found!\n", *defines_found);
}

static inline struct FunctionDecon *parse_function(struct FileBuffer *fb, unsigned int line) {
    struct FunctionDecon *rtn = calloc(1, sizeof(struct FunctionDecon));
    struct Identifier *ident;

    // Since functions can be defined over multiple lines if they have lots of arguments
    // We check for the opening curly brace and keep appending the lines if we don't find one
    char *func_str = calloc(MAX_BUFSIZE_MED, sizeof(char));
    int a, b, c;
    int z = line;

    while (1) {
        sprintf(func_str, "%s%s", func_str, consume_spaces(*(fb->buf + z)));
        a = ch_in_str('{', *(fb->buf + z));
        b = ch_in_str(';', *(fb->buf + z));
        c = ch_in_str('=', *(fb->buf + z));
        if (b || c)
            return NULL;
        else if (a)
            break;
        else  {
            z++;
            sprintf(func_str, "%s, ", func_str);
        }
    }

    // Splitting up the string
    struct FileBuffer *func_params = create_file_buffer(MAX_BUFSIZE_TINY);

    a = 0;
    int y_pos = 0;
    int x_pos = 0;
    char ch;

    while ((ch = *(func_str + a)) != '\n') {
        if (ch == ',' || ch == '(' || ch == ')') {
            func_params->y_len++;
            a++;
            y_pos++;
            x_pos = 0;
        } else {
            *(*(func_params->buf + y_pos) + x_pos) = ch;
            x_pos++;
            a++;
        }
    }

    // First in the buffer is the function identifier
    ident = string_to_identifier(*(func_params->buf));
    rtn->ident = *ident;
    free(ident);

    for (int i = 1; i < func_params->y_len; i++)
        list_push_back(&rtn->params, create_node(string_to_identifier(*(func_params->buf + i))));

    free(func_str);
    free(func_params);
    return rtn;
}


static inline struct StructDecon *parse_struct(struct FileBuffer *fb, unsigned int line) {
    // Calloc is used so the List struct inside doesn't have dangling pointers
    struct StructDecon *rtn = calloc(1, sizeof(struct StructDecon));
    struct Identifier *ident;

    // Copying the identifier
    ident = string_to_identifier(*(fb->buf + line++));
    rtn->ident = *ident;
    free(ident);

    char *str;
    while (**(fb->buf + line) != '}') {
        str = consume_spaces(*(fb->buf + line));
        if (*str == '/' || *str == '*' || *str == '\n') {
            line++;
            continue;
        } else {
            list_push_back(&rtn->members, create_node(string_to_identifier(str)));
            line++;
        }
    }

    return rtn;
}

static inline struct EnumDecon *parse_enum(struct FileBuffer *fb, unsigned int line) {
    struct EnumDecon *rtn = calloc(1, sizeof(struct EnumDecon));
    struct Identifier *ident;
    struct EnumMember *member;

    ident = string_to_identifier(*(fb->buf + line++));
    rtn->ident = *ident;
    free(ident);

    char *str;
    struct FileBuffer *decon = create_file_buffer(MAX_BUFSIZE_TINY);
    int a;
    int y_pos;
    int x_pos;
    char ch;
    int enum_num = 0;

    while (**(fb->buf + line) != '}') {
        str = consume_spaces(*(fb->buf + line));

        // Splitting up the string
        if (*str == '/' || *str == '*' || *str == '\n') {
            line++;
        } else {
            a = 0;
            y_pos = 0;
            x_pos = 0;

            decon = create_file_buffer(MAX_BUFSIZE_TINY);
            member = malloc(sizeof(struct EnumMember));
            while ((ch = *(str + a)) != '\n') {
                if (ch == ',' || ch == '=' || ch == ')') {
                    decon->y_len++;
                    a++;
                    y_pos++;
                    x_pos = 0;
                } else if (ch == ' ') {
                    a++;
                } else {
                    *(*(decon->buf + y_pos) + x_pos) = ch;
                    x_pos++;
                    a++;
                }
            }

                sprintf(member->name, "%s", *(decon->buf));
            if (decon->y_len == 1) {
                member->num = enum_num++;
            } else if (decon->y_len == 2) {
                enum_num = str_to_int(*(decon->buf + 1));
                member->num = enum_num;
            }

            list_push_back(&rtn->enums, create_node(member));

            free(decon);
            line++;
        }
    }

    free(decon);
    return rtn;
}


/**
 * This function scans the supplied string and returns an "Identifier" struct
 * Takes a length argument so I don't have to break up a string before entering it in
 * @NOTE @TODO: Can't collect function pointers yet
 * @NOTE @TODO: Can't collect arrays that look like this "arr[size]";
 * @NOTE @TODO: Can't collect "unsigned" or "signed"
 * @FIXME : I should just probably redo this function
 */
static inline struct Identifier *string_to_identifier(const char *str) {
    //printf("%s\n", str);
    struct Identifier *ident = malloc(sizeof(struct Identifier));
    struct FileBuffer *buf = create_file_buffer(MAX_BUFSIZE_TINY);
    ident->ptr   = 0;
    ident->extra = 0;

    // Split up the string
    int len = string_len(str);
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
            // Do the arr[size] check here
            // If we are at the end then it's likely the name of the variable
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

    delete_file_buffer(buf);
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
