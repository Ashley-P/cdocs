#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "html.h"



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
/**
 * Loads a html file into a buffer
 * @NOTE: Creates the buffer that is used
 * @NOTE: Assumes that the filepath is correct
 */
struct HtmlBuffer *load_html(char *fn) {
    struct HtmlBuffer *buf = create_html_buffer();

    FILE *f = fopen(fn, "r");
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

    // @NOTE : Just a quick print of what we loaded
    y_pos = 0;
    for (int i = 0; i < buf->y_len; i++)
        printf("%s", *(buf->buf + i));

    return buf;
}
