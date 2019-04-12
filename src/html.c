#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "html.h"
#include "utils.h"


// Function prototypes
static inline void add_hyperlink(struct HtmlBuffer *hb, char *docs, char *name, int pos);
static inline char *get_filename(char *fpath);


// @TODO: Have the function accept an argument for y_len
static inline struct HtmlBuffer *create_html_buffer() {
    struct HtmlBuffer *ptr = malloc(sizeof(struct HtmlBuffer));
    ptr->buf               = malloc(sizeof(char *) * MAX_BUFSIZE_MED);
    for (int i = 0; i < MAX_BUFSIZE_MED; i++)
        *(ptr->buf + i) = calloc(MAX_BUFSIZE_SMALL, sizeof(char));

    ptr->x_len      = MAX_BUFSIZE_SMALL;
    ptr->y_len      = 0;
    ptr->y_len_true = MAX_BUFSIZE_MED;

    return ptr;
}

static inline void delete_html_buffer(struct HtmlBuffer *hb) {
    for (int i = 0; i < MAX_BUFSIZE_MED; i++)
        free(*(hb->buf + i));
    free(hb->buf);
    free(hb);
}

/**
 * Loads a html file into a buffer
 * @NOTE: Creates the buffer that is used
 * @NOTE: Assumes that the filepath is correct
 */
struct HtmlBuffer *load_html(char *fn) {
    struct HtmlBuffer *buf = create_html_buffer();

    FILE *f = fopen(fn, "r");

    if (!f) {
        // Proper error logging soon
        fprintf(stderr, "\nError in load_html: File does not exist");
        delete_html_buffer(buf);
        return NULL;
    }

    char ch;
    int y_pos = 0;
    int x_pos = 0;

    while ((ch = fgetc(f)) != EOF) {
            *(*(buf->buf + y_pos) + x_pos) = ch;
            x_pos++;

        if (ch == '\n') {
            y_pos++;
            buf->y_len++;
            x_pos = 0;
        }
    }

    fclose(f);
    return buf;
}

void save_html(struct HtmlBuffer *hb, char *fn) {
    FILE *f = fopen(fn, "w");
    if (!f) {
        fprintf(stderr, "\nError in save_html: File does not exist");
        return;
    }

    int x_pos = 0;

    for (int i = 0; i < hb->y_len; i++) {
        x_pos = 0;
        while (*(*(hb->buf + i) + x_pos) != '\0') {
            fputc(*(*(hb->buf + i) + x_pos), f);
            x_pos++;
        }
        /*
        if (*(*(hb->buf + i) + x_pos) == '\n') {
            fputc('\n', f);
            x_pos = 0;
        }
        */

    }

    fclose(f);
}


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
unsigned short find_comment(const struct HtmlBuffer *hb, char *comment) {
    // For each line we consume the spaces and check if the first letters of the string match a comment
    char *c;
    for (int y = 0; y < hb->y_len; y++) {
        c = consume_spaces(*(hb->buf + y));
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
int recheck_template_positions(const struct HtmlBuffer *hb, struct TemplatePositions *tp) {
    tp->topnav  = find_comment(hb, "<!-- CDOCS:TOPNAV -->");
    tp->sidenav = find_comment(hb, "<!-- CDOCS:SIDENAV -->");
    tp->content = find_comment(hb, "<!-- CDOCS:CONTENT -->");

    tp->sidenav_files     = find_comment(hb, "<!-- CDOCS:SIDENAV:FILES -->");
    tp->sidenav_functions = find_comment(hb, "<!-- CDOCS:SIDENAV:FUNCTIONS -->");
    tp->sidenav_structs   = find_comment(hb, "<!-- CDOCS:SIDENAV:STRUCTS -->");
    tp->sidenav_defines   = find_comment(hb, "<!-- CDOCS:SIDENAV:DEFINES -->");
    tp->sidenav_enums     = find_comment(hb, "<!-- CDOCS:SIDENAV:ENUMS -->");

    // @TODO: Better handling
    if (tp->topnav == 0 || tp->sidenav == 0 || tp->content == 0 || tp->sidenav_files == 0 ||
        tp->sidenav_functions == 0 || tp->sidenav_structs == 0 || tp->sidenav_defines == 0 ||
        tp->sidenav_enums     == 0) {
        fprintf(stderr, "Couldn't find specific comment");
        return 0;
    }

    return 1;
}

/**
 * This edits the loaded template and customizes it to fit the source files
 * @TODO: Proper indenting
 */
void gen_template(struct HtmlBuffer *hb, struct TemplatePositions *tp, struct DirectoryBuffer *db, char *docs) {
    // Generating the file section in sidenav
    // @TODO: We need to match headers with the source files so we can combine their pages
    // Loop over the directory buffer
    for (int i = 0; i < db->y_len; i++) {
        char *fname = get_filename(*(db->buf + i));
        add_hyperlink(hb, docs, fname, tp->sidenav_files + 1);
        free(fname);
    }

    // Save the custom template to the docs folder
    char fn[MAX_BUFSIZE_MED];
    sprintf(fn, "%stemplates\\template.html", docs);
    save_html(hb, fn);
}

/**
 * Creates a hyper link at the line provided in the buffer
 */
static inline void add_hyperlink(struct HtmlBuffer *hb, char *docs, char *name, int pos) {
    shift_pointers_right((void **) hb->buf, MAX_BUFSIZE_MED, 1, pos);
    *(hb->buf + pos) = calloc(MAX_BUFSIZE_SMALL, sizeof(char));

    // Construct the string
    sprintf(*(hb->buf + pos), "<a href=\"%s%s.html\">%s</a>\n", docs, name, name);
    hb->y_len++;
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

