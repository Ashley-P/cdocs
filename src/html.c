#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "html.h"
#include "utils.h"



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

static inline void delete_html_buffer(struct HtmlBuffer *buf) {
    for (int i = 0; i < MAX_BUFSIZE_MED; i++)
        free(*(buf->buf + i));
    free(buf->buf);
    free(buf);
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

void save_html(struct HtmlBuffer buf, char *fn) {}


// Returns a pointer that is a the first non-whitespace character in the string
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
unsigned short find_comment(struct HtmlBuffer *buf, char *comment) {
    // For each line we consume the spaces and check if the first letters of the string match a comment
    char *c;
    for (int y = 0; y < buf->y_len; y++) {
        c = consume_spaces(*(buf->buf + y));
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
int recheck_template_positions(struct HtmlBuffer *buf, struct TemplatePositions *tp) {
    tp->topnav  = find_comment(buf, "<!-- CDOCS:TOPNAV -->");
    tp->sidenav = find_comment(buf, "<!-- CDOCS:SIDENAV -->");
    tp->content = find_comment(buf, "<!-- CDOCS:CONTENT -->");

    tp->sidenav_files     = find_comment(buf, "<!-- CDOCS:SIDENAV:FILES -->");
    tp->sidenav_functions = find_comment(buf, "<!-- CDOCS:SIDENAV:FUNCTIONS -->");
    tp->sidenav_structs   = find_comment(buf, "<!-- CDOCS:SIDENAV:STRUCTS -->");
    tp->sidenav_defines   = find_comment(buf, "<!-- CDOCS:SIDENAV:DEFINES -->");
    tp->sidenav_enums     = find_comment(buf, "<!-- CDOCS:SIDENAV:ENUMS -->");

    if (tp->topnav == 0 || tp->sidenav == 0 || tp->content == 0 || tp->sidenav_files == 0 ||
        tp->sidenav_functions == 0 || tp->sidenav_structs == 0 || tp->sidenav_defines == 0 ||
        tp->sidenav_enums     == 0) {
        fprintf(stderr, "Couldn't find specific comment");
        return 0;
    }

    return 1;
}

void gen_template(struct HtmlBuffer *buf, struct TemplatePositions *tp) {
}
