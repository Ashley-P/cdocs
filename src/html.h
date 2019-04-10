#ifndef HTML_H
#define HTML_H

struct HtmlBuffer {
    char **buf;
    unsigned short x_len;
    unsigned short y_len;
    unsigned short y_len_true; // The malloc'd sized of buf
};

// Extern Functions
struct HtmlBuffer *load_html(char *fn);

#endif
