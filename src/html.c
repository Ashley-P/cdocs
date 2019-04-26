#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "docs.h"
#include "files.h"
#include "html.h"
#include "utils.h"


// Function prototypes
static inline void add_hyperlink(struct FileBuffer *fb, char *docs, char *name, int pos);
static inline char *get_filename(char *fpath);







/**
 * Finds the html comment <!-- CDOCS:<comment> --> and returns the line number
 * 0 on failure
 */
unsigned short find_comment(const struct FileBuffer *fb, char *comment) {
    // For each line we consume the spaces and check if the first letters of the string match a comment
    char *c;
    for (int y = 0; y < fb->y_len; y++) {
        c = consume_spaces(*(fb->buf + y));
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
int recheck_template_positions(const struct FileBuffer *fb, struct TemplatePositions *tp) {
    tp->topnav  = find_comment(fb, "<!-- CDOCS:TOPNAV -->");
    tp->sidenav = find_comment(fb, "<!-- CDOCS:SIDENAV -->");
    tp->content = find_comment(fb, "<!-- CDOCS:CONTENT -->");

    tp->sidenav_header    = find_comment(fb, "<!-- CDOCS:SIDENAV:HEADER -->");
    tp->sidenav_source    = find_comment(fb, "<!-- CDOCS:SIDENAV:SOURCE -->");
    tp->sidenav_functions = find_comment(fb, "<!-- CDOCS:SIDENAV:FUNCTIONS -->");
    tp->sidenav_structs   = find_comment(fb, "<!-- CDOCS:SIDENAV:STRUCTS -->");
    tp->sidenav_defines   = find_comment(fb, "<!-- CDOCS:SIDENAV:DEFINES -->");
    tp->sidenav_enums     = find_comment(fb, "<!-- CDOCS:SIDENAV:ENUMS -->");

    // @TODO: Better handling
    if (tp->topnav == 0 || tp->sidenav == 0 || tp->content == 0 || tp->sidenav_header == 0 ||
        tp->sidenav_source == 0 || tp->sidenav_functions == 0 || tp->sidenav_structs == 0 ||
        tp->sidenav_defines == 0 || tp->sidenav_enums     == 0) {
        fprintf(stderr, "Couldn't find specific comment");
        return 0;
    }

    return 1;
}

/**
 * This edits the loaded template and customizes it to fit the source files
 * @TODO: Proper indenting
 * Directory buffer is needed for the filenames
 */
void gen_template(struct FileBuffer *template_fb, struct TemplatePositions *tp, struct DirectoryBuffer *db, 
        struct List *functions, struct List *structs, struct List *enums, struct List *defines, char *doc_dir) {
    // Generating the file section in sidenav
    // @TODO: We need to match headers with the source files so we can combine their pages
    // Loop over the directory buffer
    int len;
    char *fname;
    int i;

    // Files
    for (i = 0; i < db->y_len; i++) {
        fname = get_filename(*(db->buf + i));
        // Check if the file is a header or a source file
        len = string_len(fname);
        if (*(fname + len - 1) == 'h')
            add_hyperlink(template_fb, doc_dir, fname, tp->sidenav_header + 1);
        else if (*(fname + len - 1) == 'c')
            add_hyperlink(template_fb, doc_dir, fname, tp->sidenav_source + 1);
        free(fname);
    }

    // Have to recheck the template every time
    recheck_template_positions(template_fb, tp);

    // Functions
    struct Node *node = functions->head;
    struct FunctionDecon *f;

    while (node) {
        f = node->data;
        add_hyperlink(template_fb, doc_dir, f->ident.name, tp->sidenav_functions + 1);
        node = node->next;
    }

    // Have to recheck the template every time
    recheck_template_positions(template_fb, tp);

    // Structs
    node = structs->head;
    struct StructDecon *s;

    while (node) {
        s = node->data;
        add_hyperlink(template_fb, doc_dir, s->ident.name, tp->sidenav_structs + 1);
        node = node->next;
    }

    // Have to recheck the template every time
    recheck_template_positions(template_fb, tp);

    // Enums
    node = enums->head;
    struct EnumDecon *e;

    while (node) {
        e = node->data;
        add_hyperlink(template_fb, doc_dir, e->ident.name, tp->sidenav_enums + 1);
        node = node->next;
    }

    // Have to recheck the template every time
    recheck_template_positions(template_fb, tp);

    /* Defines aren't implemented yet
    // Defines
    node = defines->head;
    struct DefineDecon *d;

    while (node) {
        d = node->data;
        add_hyperlink(template_fb, doc_dir, d->ident.name, tp->sidenav_defines + 1);
        node = node->next;
    }
    */


    // Save the custom template to the docs folder
    char fn[MAX_BUFSIZE_MED];
    sprintf(fn, "%stemplates\\template.html", doc_dir);
    save_file(template_fb, fn);
}

/**
 * Creates a hyper link at the line provided in the buffer
 */
static inline void add_hyperlink(struct FileBuffer *fb, char *docs, char *name, int pos) {
    char *line = calloc(MAX_BUFSIZE_MED, sizeof(char));
    sprintf(line, "<a href=\"%s%s.html\">%s</a>\n", docs, name, name);
    buffer_insert_line(fb, line, pos);
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

