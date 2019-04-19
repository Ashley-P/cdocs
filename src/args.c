#ifdef __unix__
# error Unix is unsupported
#elif defined _WIN32
# include <io.h>
# include <windows.h>
#endif

#include <stdio.h>
#include "args.h"
#include "docs.h"
#include "files.h"
#include "ll.h"
#include "html.h"
#include "utils.h"



// Function Prototypes
void c_new(char *source, char *docs);
void c_regen(char *config);
void setup_docgen(char *config, struct DocGen *docgen);


#define COMMAND_NUM 2
const static char *command_names[] = {"new",
                                      "regen",};

void parse_arguments(int argc, char **argv) {
    // Just return if there is no argument supplied
    if (argc < 2) {
        return;
    }

    // First is the basic check to see if the argv[0] is a valid command name
    if (!string_in_strings(argv[1], command_names, COMMAND_NUM)) {
        fprintf(stderr, "Invalid command \"%s\"", argv[1]);
        return;
    }

    /* new */
    if (string_cmp(argv[1], command_names[0])) {
        if (argc != 4) {
            fprintf(stderr, "Incorrect number of arguments");
            return;
        }
        c_new(argv[2], argv[3]);

    /* regen */
    } else if (string_cmp(argv[1], command_names[1])) {
        if (argc != 3) {
            fprintf(stderr, "Incorrect number of arguments");
            return;
        }
        c_regen(argv[2]);
    }
}

/**
 * Creates a config file based on the two arguments
 * Arg 1: Directory where the source files are
 * Arg 2: Directory where the documentation should appear
 * @NOTE : The config file is placed in the docs directory
 */
void c_new(char *source, char *docs) {
#ifdef _WIN32
    // @TODO: Check if the path is relative or absolute
    // Convert both directory names to the full path
    char cur_dir[MAX_BUFSIZE_MED];
    char new_src[MAX_BUFSIZE_MED];
    char new_doc[MAX_BUFSIZE_MED];
    GetCurrentDirectory(MAX_BUFSIZE_MED, cur_dir);

    if (*(source) == '.')
        sprintf(new_src, "%s%s", cur_dir, source + 1);
    else
        sprintf(new_src, "%s%s", cur_dir, source);

    if (*(docs) == '.')
        sprintf(new_doc, "%s%s", cur_dir, docs + 1);
    else
        sprintf(new_doc, "%s%s", cur_dir, docs);


    // Check both directory names
    if (GetFileAttributesA(new_src) == INVALID_FILE_ATTRIBUTES ||
        !(GetFileAttributesA(new_src) & FILE_ATTRIBUTE_DIRECTORY)) {
        fprintf(stderr, "Invalid directory name \"%s\"", new_src);
        return;
    } else if (GetFileAttributesA(new_doc) == INVALID_FILE_ATTRIBUTES ||
        !(GetFileAttributesA(new_doc) & FILE_ATTRIBUTE_DIRECTORY)) {
        fprintf(stderr, "Invalid directory name \"%s\"", new_doc);
        return;
    }
#endif

    // Create a config file in docs with both filenames in
    char path[MAX_BUFSIZE_MED];
    sprintf(path, "%s/cdocs.cfg", new_doc);

#ifdef _WIN32
    // Checking if the file exists
    char input[1];
    if (!(GetFileAttributesA(new_src) == INVALID_FILE_ATTRIBUTES)) {
        do {
            printf("File \"%s\" already exists, would you like to overwrite? \n[Y]es [N]o\n", path);
            scanf(" %c", input);
        } while (input[0] != 'y' && input[0] != 'Y' && input[0] != 'n' && input[0] != 'N');

        if (input[0] == 'n' || input[0] == 'N') // @TODO: Should create a "to_lower" function
            return;
    }
    
    // Checking and creating the extra directories
    char path2[MAX_BUFSIZE_MED];

    // html directory
    sprintf(path2, "%s/html", new_doc);
    if (GetFileAttributesA(path2) == INVALID_FILE_ATTRIBUTES) {
        _mkdir(path2);
    }

    // templates directory
    sprintf(path2, "%s/templates", new_doc);
    if (GetFileAttributesA(path2) == INVALID_FILE_ATTRIBUTES) {
        _mkdir(path2);
    }

#endif

    FILE *f = fopen(path, "w+");
    fprintf(f, "SourceDirectory=%s\n", new_src);
    fprintf(f, "DocsDirectory=%s\n", new_doc);
    fclose(f);
    printf("\nConfig file created at \"%s\"", path);
}

/**
 * Creates the documentation based the config file
 */
void c_regen(char *config) {
#ifdef _WIN32
    // Check the path to the config file is correct
    if (GetFileAttributesA(config) == FILE_ATTRIBUTE_DIRECTORY ||
        GetFileAttributesA(config) == INVALID_FILE_ATTRIBUTES) {
        fprintf(stderr, "Invalid file path \"%s\"", config);
        return;
    }

    //char bin_dir[MAX_BUFSIZE_MED];
    char *bin_dir = calloc(MAX_BUFSIZE_MED, sizeof(char));
    GetModuleFileNameA(NULL, bin_dir, MAX_BUFSIZE_MED);
#endif


    // Strip the exe name away so we can use the filepath to access
    int strlen = string_len(bin_dir);
    int slash_pos = strlen;

    while (*(bin_dir + slash_pos) != '\\')
        slash_pos--;

    *(bin_dir + slash_pos) = '\0';
    //char template[MAX_BUFSIZE_MED];
    //char js[MAX_BUFSIZE_MED];
    //char css[MAX_BUFSIZE_MED];
    char *template = calloc(MAX_BUFSIZE_MED, sizeof(char));
    char *js       = calloc(MAX_BUFSIZE_MED, sizeof(char));
    char *css      = calloc(MAX_BUFSIZE_MED, sizeof(char));
    sprintf(template, "%s\\resources\\template.html", bin_dir);
    sprintf(js,       "%s\\resources\\dropdown.js",   bin_dir);
    sprintf(css,      "%s\\resources\\styles.css",    bin_dir);

    free(bin_dir);
    // Setting up DocGen
    struct DocGen *docgen = malloc(sizeof(struct DocGen));
    setup_docgen(config, docgen);


    // Copying the non-edited files over to the docs folder
    //char js_path[MAX_BUFSIZE_MED];
    //char css_path[MAX_BUFSIZE_MED];
    char *js_path  = calloc(MAX_BUFSIZE_MED, sizeof(char));
    char *css_path = calloc(MAX_BUFSIZE_MED, sizeof(char));
    sprintf(js_path, "%stemplates\\dropdown.js", docgen->doc_dir);
    sprintf(css_path, "%stemplates\\styles.css", docgen->doc_dir);

#ifdef _WIN32
    CopyFile(js,  js_path, 0);
    CopyFile(css, css_path, 0);
#endif

    // Loading the template for pre-editing before generating the documents
    struct FileBuffer *hb = load_file(template);
    struct TemplatePositions *tp = malloc(sizeof(struct TemplatePositions));
    recheck_template_positions(hb, tp);

    // Scanning the files
    struct DirectoryBuffer *db = scan_directory(docgen->src_dir);
    if (!db) return;

    // The template needs atleast the names of functions etc so we just deconstruct the whole thing
    // Here and pass it to gen_template
    // We should only search for functions in .c files
    struct Node *functions = NULL;

    for (int i = 0; i < db->y_len; i++) {
        if (*(*(db->buf + i) + string_len(*(db->buf + i)) - 1) == 'h')
            continue;
        else {
            struct Node *temp = scan_file_functions(*(db->buf + i));
            list_push_back(functions, temp);
        }
    }

    printf("\n");

    // Gen the template
    //gen_template(hb, tp, db, docgen->doc_dir);
    printf("Valid!");

    free(template);
    free(js);
    free(css);
    free(js_path);
    free(css_path);
}

char *reverse_docgenopt_enum(enum DocGenOpt a) {
    switch (a) {
        case DG_INVALID:   return "DG_INVALID";
        case DG_EQUALS:    return "=";
        case DG_ADDEQUALS: return "+=";
    }

    return "";
}

/**
 *  Sets up the docgen struct
 */
void setup_docgen(char *config, struct DocGen *docgen) {
    FILE *f = fopen(config, "r");

    char read[MAX_BUFSIZE_MED];
    char cfg[MAX_BUFSIZE_MED];
    char opt[MAX_BUFSIZE_MED];
    enum DocGenOpt dgo;
    int a;

    while ((a = fscanf(f, "%s", read)) != EOF) {
        int x = str_len_to_ch(read, '+');
        int y = str_len_to_ch(read, '=');

        int len = (x < y) ? x : y;

        str_cpy2(read, cfg, len);

        // Checking operators
        if (*(read + len) == '+' && *(read + len + 1) == '=') {
            dgo = DG_ADDEQUALS;
            str_cpy(read + len + 2, opt);

        } else if (*(read + len) == '=') {
            dgo = DG_EQUALS;
            str_cpy(read + len + 1, opt);

        } else {
            fprintf(stderr, "Invalid string \"%s\"", read);
            exit(0);
        }

        // Filling out the struct
        if (string_cmp(cfg, "SourceDirectory")) {
            if (dgo != DG_EQUALS) {
                fprintf(stderr, "Incorrect Operator, expected \"=\" got \"%s\"\n",
                        reverse_docgenopt_enum(dgo));
            } else 
                str_cpy(opt, docgen->src_dir);

        } else if (string_cmp(cfg, "DocsDirectory")) {
            if (dgo != DG_EQUALS) {
                fprintf(stderr, "Incorrect Operator, expected \"=\" got \"%s\"\n",
                        reverse_docgenopt_enum(dgo));
            } else 
                str_cpy(opt, docgen->doc_dir);
        }
    }

    // Testing stuff
    //printf("%s\n", docgen->src_dir);
    //printf("%s\n", docgen->doc_dir);

    // Check that the mandatory members are filled
    if (*(docgen->src_dir) == '\0') {
        fprintf(stderr, "Error: No SourceDirectory variable in \"%s\"", config);
        exit(0);
    } else if (*(docgen->doc_dir) == '\0') {
        fprintf(stderr, "Error: No DocsDirectory variable in \"%s\"", config);
        exit(0);
    }

    fclose(f);
};
