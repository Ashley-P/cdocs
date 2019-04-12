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
void save_html(struct HtmlBuffer *hb, char *fn);
unsigned short find_comment(const struct HtmlBuffer *hbf, char *comment);
int recheck_template_positions(const struct HtmlBuffer *hb, struct TemplatePositions *tp);
void gen_template(struct HtmlBuffer *hb, struct TemplatePositions *tp, struct DirectoryBuffer *db, char *docs);

#endif
