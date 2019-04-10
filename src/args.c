#ifdef __unix__
# error Unix is unsupported
#elif defined _WIN32
# include <io.h>
# include <windows.h>
#endif

#include <stdio.h>
#include "defs.h"
#include "html.h"
#include "utils.h"



// Function Prototypes
void c_new(char *source, char *docs);
void c_regen(char *config);


#define COMMAND_NUM 2
const static char *command_names[] = {"new",
                                      "regen",};

void parse_arguments(int argc, char** argv) {
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
    // Check both directory names
#ifdef _WIN32
    if (GetFileAttributesA(source) == INVALID_FILE_ATTRIBUTES ||
        !(GetFileAttributesA(source) & FILE_ATTRIBUTE_DIRECTORY)) {
        fprintf(stderr, "Invalid directory name \"%s\"", source);
        return;
    } else if (GetFileAttributesA(docs) == INVALID_FILE_ATTRIBUTES ||
        !(GetFileAttributesA(docs) & FILE_ATTRIBUTE_DIRECTORY)) {
        fprintf(stderr, "Invalid directory name \"%s\"", docs);
        return;
    }
#endif

    // Create a config file in docs with both filenames in
    char path[MAX_BUFSIZE_MED];
    sprintf(path, "%s/cdocs.cfg", docs);

#ifdef _WIN32
    // Checking if the file exists
    char input[1];
    if (!(GetFileAttributesA(source) == INVALID_FILE_ATTRIBUTES)) {
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
    sprintf(path2, "%s/html", docs);
    if (GetFileAttributesA(path2) == INVALID_FILE_ATTRIBUTES) {
        _mkdir(path2);
    }

    // templates directory
    sprintf(path2, "%s/templates", docs);
    if (GetFileAttributesA(path2) == INVALID_FILE_ATTRIBUTES) {
        _mkdir(path2);
    }

#endif

    FILE *f = fopen(path, "w+");
    fprintf(f, "SourceDirectory=%s\n", source);
    fprintf(f, "DocsDirectory=%s\n", docs);
    fclose(f);
    printf("\nConfig file created at \"%s\"", path);
}

/**
 * Creates the documentation based the config file
 */
void c_regen(char *config) {
#ifdef _WIN32
    // Check the path to the config file is correct
    if (GetFileAttributesA(config) == INVALID_FILE_ATTRIBUTES) {
        printf("Invalid file path \"%s\"", config);
        return;
    }

    char template[MAX_BUFSIZE_MED];
    GetModuleFileNameA(NULL, template, MAX_BUFSIZE_MED);
#endif


    // Strip the exe name away so we can use the filepath to access
    int strlen = string_len(template);
    int slash_pos = strlen;

    while (*(template + slash_pos) != '\\')
        slash_pos--;

    *(template + slash_pos) = '\0';
    sprintf(template, "%s\\resources\\template.html", template);


    // Loading the template for pre-editing before generating the documents
    struct HtmlBuffer *buf = load_html(template);
    struct TemplatePositions *template_positions = malloc(sizeof(struct TemplatePositions));

    printf("Valid!");
}

