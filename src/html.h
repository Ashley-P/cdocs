#ifndef HTML_H
#define HTML_H

// Records the positions of the comments in the template html file
struct TemplatePositions {
    unsigned short topnav;
    unsigned short sidenav;
    unsigned short content;

    unsigned short sidenav_files;
    unsigned short sidenav_functions;
    unsigned short sidenav_structs;
    unsigned short sidenav_defines;
    unsigned short sidenav_enums;
};

// Extern Functions
struct HtmlBuffer *load_html(char *fn);
void save_html(struct HtmlBuffer buf, char *fn);
unsigned short find_comment(struct HtmlBuffer *buf, char *comment);
int recheck_template_positions(struct HtmlBuffer *buf, struct TemplatePositions *tp);

#endif
