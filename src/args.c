#ifdef __unix__
# error Unix is unsupported
#elif defined _WIN32
# include <windows.h>
#endif

#include <stdio.h>
#include "utils.h"



#define COMMAND_NUM 2
static char *command_names[] = {"new",
                                "regen",};

/**
 * Creates a config file based on the two arguments
 * Arg 1: Directory where the source files are
 * Arg 2: Directory where the documentation should appear
 * @NOTE : The config file is placed in the docs directory
 */
void c_new(char *source, char *docs) {
    
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

    printf("Valid!");
}
