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
void save_html(struct HtmlBuffer buf, char *fn);
unsigned short find_comment(struct HtmlBuffer *buf, char *comment);

// Extern Variables
extern unsigned short template_cdocs_topnav;
extern unsigned short template_cdocs_sidenav;
extern unsigned short template_cdocs_content;

#endif
