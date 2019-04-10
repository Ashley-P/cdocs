#ifndef ARGS_H
#define ARGS_H

#include "defs.h"


/**
 * Some options for docgen
 */
enum DocGenOpt {
    DG_INVALID = 0,
    DG_EQUALS,
    DG_ADDEQUALS,
};

/**
 * A struct with things to help create the documentation proper
 */
struct DocGen {
    char src_dir[MAX_BUFSIZE_MED];
    char doc_dir[MAX_BUFSIZE_MED];
};

/* Extern Functions */
void parse_arguments(int argc, char** argv);


#endif
