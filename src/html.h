#ifndef HTML_H
#define HTML_H

// Records the positions of the comments in the template html file
struct TemplatePositions {
    unsigned short topnav;
    unsigned short sidenav;
    unsigned short content;

    unsigned short sidenav_header;
    unsigned short sidenav_source;
    unsigned short sidenav_functions;
    unsigned short sidenav_structs;
    unsigned short sidenav_defines;
    unsigned short sidenav_enums;
};

// Extern Functions
unsigned short find_comment(const struct FileBuffer *hbf, char *comment);
int recheck_template_positions(const struct FileBuffer *hb, struct TemplatePositions *tp);
void gen_template(struct FileBuffer *hb, struct TemplatePositions *tp, struct DirectoryBuffer *db, char *docs);

#endif
