#include <stdio.h>



int main(int argc, char** argv) {
    FILE *fp = _wfopen(L".\\htmltest.html", L"w");

    fprintf(fp,
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "<title>TEST</title>\n"
            "</head>\n"
            "<body>\n"
            "<h1>Header 1</h1>\n"
            "<p>This is a paragraph</p>\n"
            "</body>\n"
            "</html>\n"
           );

    fclose(fp);
}
