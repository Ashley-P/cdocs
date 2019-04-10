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
        printf("\nError in load_html: File does not exist");
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

    return buf;
}
