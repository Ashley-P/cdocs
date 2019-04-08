#ifdef __unix__
# error Unix is unsupported
#elif defined _WIN32
# include <io.h>
# include <windows.h>
#endif

#include <stdio.h>
#include "defs.h"
#include "utils.h"



#define COMMAND_NUM 2
static char *command_names[] = {"new",
                                "regen",};

/* Function Prototypes */
void *construct_filename(char *buf, char *path);

/**
 * Creates a config file based on the two arguments
 * Arg 1: Directory where the source files are
 * Arg 2: Directory where the documentation should appear
 * @NOTE : The config file is placed in the docs directory
 */
void c_new(char *source, char *docs) {
    // Check both directory names
    // Windows only
#ifdef _WIN32
    if (GetFileAttributesA(source) == INVALID_FILE_ATTRIBUTES ||
        !(GetFileAttributesA(source) & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("Invalid directory name \"%s\"", source);
        return;
    } else if (GetFileAttributesA(docs) == INVALID_FILE_ATTRIBUTES ||
        !(GetFileAttributesA(docs) & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("Invalid directory name \"%s\"", docs);
        return;
    }
#endif

    // Create a config file in docs with both filenames in
    // @NOTE: No check right now to see if the file exists
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
#endif

    // Check if docs has a slash at the end
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
}

void parse_arguments(int argc, char** argv) {
    // First is the basic check to see if the argv[0] is a valid command name
    if (!string_in_strings(argv[1], command_names, COMMAND_NUM)) {
        printf("Invalid command \"%s\"", argv[1]);
        return;
    }

    /* new */
    if (string_cmp(argv[1], command_names[0])) {
        if (argc != 4) {
            printf("Incorrect number of arguments");
            return;
        }
        c_new(argv[2], argv[3]);
    /* regen */
    } else if (string_cmp(argv[1], command_names[1])) {}
}
