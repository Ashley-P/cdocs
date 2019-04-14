#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "docs.h"
#include "files.h"
#include "html.h"
#include "utils.h"


// Function prototypes
static inline void add_hyperlink(struct FileBuffer *fb, char *docs, char *name, int pos);
static inline char *get_filename(char *fpath);






// Returns a pointer that is at the first non-whitespace character in the string
static inline char *consume_spaces(char *str) {
    char *tmp = str;
    while (*tmp == ' ')
        ++tmp;

    return tmp;
}

/**
 * Finds the html comment <!-- CDOCS:<comment> --> and returns the line number
 * 0 on failure
 */
unsigned short find_comment(const struct FileBuffer *fb, char *comment) {
    // For each line we consume the spaces and check if the first letters of the string match a comment
    char *c;
    for (int y = 0; y < fb->y_len; y++) {
        c = consume_spaces(*(fb->buf + y));
        if (string_cmp2(c, comment, string_len(comment) - 1))
            return y;
    }

    return 0;
}

/**
 * Checks the buffer for the cdocs comments
 * returns 0 on failure and 1 on success
 * @TODO: Proper error checking later
 */
int recheck_template_positions(const struct FileBuffer *fb, struct TemplatePositions *tp) {
    tp->topnav  = find_comment(fb, "<!-- CDOCS:TOPNAV -->");
    tp->sidenav = find_comment(fb, "<!-- CDOCS:SIDENAV -->");
    tp->content = find_comment(fb, "<!-- CDOCS:CONTENT -->");

    tp->sidenav_header    = find_comment(fb, "<!-- CDOCS:SIDENAV:HEADER -->");
    tp->sidenav_source    = find_comment(fb, "<!-- CDOCS:SIDENAV:SOURCE -->");
    tp->sidenav_functions = find_comment(fb, "<!-- CDOCS:SIDENAV:FUNCTIONS -->");
    tp->sidenav_structs   = find_comment(fb, "<!-- CDOCS:SIDENAV:STRUCTS -->");
    tp->sidenav_defines   = find_comment(fb, "<!-- CDOCS:SIDENAV:DEFINES -->");
    tp->sidenav_enums     = find_comment(fb, "<!-- CDOCS:SIDENAV:ENUMS -->");

    // @TODO: Better handling
    if (tp->topnav == 0 || tp->sidenav == 0 || tp->content == 0 || tp->sidenav_header == 0 ||
        tp->sidenav_source == 0 || tp->sidenav_functions == 0 || tp->sidenav_structs == 0 ||
        tp->sidenav_defines == 0 || tp->sidenav_enums     == 0) {
        fprintf(stderr, "Couldn't find specific comment");
        return 0;
    }

    return 1;
}

/**
 * This edits the loaded template and customizes it to fit the source files
 * @TODO: Proper indenting
 */
void gen_template(struct FileBuffer *fb, struct TemplatePositions *tp, struct DirectoryBuffer *db, char *docs) {
    // Generating the file section in sidenav
    // @TODO: We need to match headers with the source files so we can combine their pages
    // Loop over the directory buffer
    int len;
    char *fname;

    for (int i = 0; i < db->y_len; i++) {
        fname = get_filename(*(db->buf + i));
        // Check if the file is a header or a source file
        len = string_len(fname);
        if (*(fname + len - 1) == 'h')
            add_hyperlink(fb, docs, fname, tp->sidenav_header + 1);
        else if (*(fname + len - 1) == 'c')
            add_hyperlink(fb, docs, fname, tp->sidenav_source + 1);
        free(fname);
    }


    // Recheck the template after every insertion
    recheck_template_positions(fb, tp);

    // Adding functions to the template
    // @FIXME @TODO: Should probably refactor and put elsewhere
    // TODO: Arrange the functions into alphabetical order
    char func[MAX_BUFSIZE_MINI];
    struct FileBuffer *functions;
    int name_start;
    int name_end;
    // Resusing len
    for (int k = 0; k < db->y_len; k++) {
        // Scan each file
        // Skip headers
        if (*(*(db->buf + k) + string_len(*(db->buf + k))) == 'h')
            continue;
        functions = scan_file_functions(*(db->buf + k));

        for (int i = 0; i < functions->y_len; i++) {
            // Scan each line 
            int len = string_len(*(functions->buf + i));
            // @FIXME : Could be refactored
            while (*(*(functions->buf + i) + len) != '(') {
                len--;
            }

            // When we get to the opening parantheses we set name_end;
            name_end = len;

            while (*(*(functions->buf + i) + len) != ' ' &&
                   *(*(functions->buf + i) + len) != '*') {
                len--;
            }
            name_start = ++len;
            str_cpy2(*(functions->buf + i) + name_start, func, name_end - name_start);

            printf("%s\n", func);
            add_hyperlink(fb, docs, func, tp->sidenav_functions + 1);
        }

        free(functions);
    }

    // Save the custom template to the docs folder
    char fn[MAX_BUFSIZE_MED];
    sprintf(fn, "%stemplates\\template.html", docs);
    save_file(fb, fn);
}

/**
 * Creates a hyper link at the line provided in the buffer
 */
static inline void add_hyperlink(struct FileBuffer *fb, char *docs, char *name, int pos) {
    shift_pointers_right((void **) fb->buf, MAX_BUFSIZE_MED, 1, pos);
    *(fb->buf + pos) = calloc(MAX_BUFSIZE_SMALL, sizeof(char));

    // Construct the string
    sprintf(*(fb->buf + pos), "<a href=\"%s%s.html\">%s</a>\n", docs, name, name);
    fb->y_len++;
}

/**
 * Strips the rest of the filepath and leaves the file name
 */
static inline char *get_filename(char *fpath) {
    char *fname = calloc(MAX_BUFSIZE_MED, sizeof(char));
    int len = string_len(fpath);
    // Work backwards from the string and stop at the first slash
    while (*(fpath + len) != '\\' && *(fpath + len) != '/') len--;

    str_cpy(fpath + len + 1, fname);
    return fname;
}

