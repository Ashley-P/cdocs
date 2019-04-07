#include <stdio.h>
#include "args.h"


int main(int argc, char** argv) {
    // Interpret the arguments that come in
    parse_arguments(argc, argv);



    FILE *fp = _wfopen(L".\\htmltest.html", L"w");

    fprintf(fp,
            "<!DOCTYPE html>\n"
            "<html>\n"
            "\t<head>\n"
            "\t\t<title>TEST</title>\n"
            "\t</head>\n"
            "\t<body>\n"
            "\t\t<h1>Header 1</h1>\n"
            "\t\t<p>This is a paragraph</p>\n"
            "\t</body>\n"
            "</html>\n"
           );

    fclose(fp);
}
