#include <stdio.h>
#include <stdlib.h>

#include "files.h"
//#include "html.h"


static inline int lines_in_file(const char *fn);

// @TODO: Have the function accept an argument for y_len
struct FileBuffer *create_file_buffer(int y_len) {
    struct FileBuffer *ptr = malloc(sizeof(struct FileBuffer));
    ptr->buf               = malloc(sizeof(char *) * y_len);
    for (int i = 0; i < y_len; i++)
        *(ptr->buf + i) = calloc(MAX_BUFSIZE_SMALL, sizeof(char));

    ptr->x_len      = MAX_BUFSIZE_SMALL;
    ptr->y_len      = 0;
    ptr->y_len_true = y_len;

    return ptr;
}

void delete_file_buffer(struct FileBuffer *fb) {
    for (int i = 0; i < fb->y_len_true; i++)
        free(*(fb->buf + i));
    free(fb->buf);
    free(fb);
}

/**
 * Loads a text file into a buffer
 * @NOTE: Creates the buffer that is used
 * @NOTE: Assumes that the filepath is correct
 */
struct FileBuffer *load_file(char *fn) {
    struct FileBuffer *buf = create_file_buffer(lines_in_file(fn));

    FILE *f = fopen(fn, "r");

    if (!f) {
        // Proper error logging soon
        fprintf(stderr, "\nError in load_file: File does not exist");
        delete_file_buffer(buf);
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

void save_file(struct FileBuffer *hb, char *fn) {
    FILE *f = fopen(fn, "w");
    if (!f) {
        fprintf(stderr, "\nError in save_file: File does not exist");
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

static inline int lines_in_file(const char *fn) {
    FILE *f = fopen(fn, "r");
    if (!f) {
        fclose(f);
        return -1;
    }

    int cnt = 0;
    char ch;
    while ((ch = fgetc(f)) != EOF) {
        if (ch == '\n') cnt++;
    }

    fclose(f);
    return cnt;
}
