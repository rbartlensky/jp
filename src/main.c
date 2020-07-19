#include <stdio.h>
#include <string.h>

#include "json.h"

static char *USAGE = "usage: -f; --file <filename>\n";

char *parse_command_line_args(char **argv) {
        if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--file") == 0) {
                return argv[2];
        }
        return NULL;
}

int main(int argc, char **argv) {
        char *filename;
        FILE *fd;
        int status = 0;

        if (argc != 3) {
                fprintf(stderr, "%s", USAGE);
                return 1;
        }
        filename = parse_command_line_args(argv);
        if (!filename) {
                fprintf(stderr, "%s", USAGE);
                return 1;
        }

        fd = fopen(filename, "r");
        if (!fd) {
                fprintf(stderr, "Failed to read file: %s\n", filename);
                return 1;
        }

        status = jp_check(fd);
        fclose(fd);

        if (status == JP_BAD_JSON) {
                return 1;
        }

        return 0;
}
