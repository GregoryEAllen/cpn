
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>


int main(int argc, char **argv) {
    int seperator = '\n';
    bool show_help = false;
    int exit_status = 0;
    while (true) {
        int c = getopt(argc, argv, "hn0s:");
        if (c == -1) break;
        switch (c) {
        case 'n':
            seperator = -1;
            break;
        case '0':
            seperator = 0;
            break;
        case 's':
            seperator = *optarg;
            break;
        case 'h':
            show_help = true;
            break;
        default:
            show_help = true;
            exit_status++;
            break;
        }
    }
    if (show_help) {
        fprintf(stderr, "Usage: %s [options] [path]\n", *argv);
        fprintf(stderr, "\t-n\tOutput nothing after the path.\n"
                "\t-h\tThis message\n"
                "\t-0\tUse a null byte after path.\n"
                "\t-s c\tUse c after path.\n");
        return exit_status;
    }
    bool first = true;
    while (optind < argc && exit_status == 0) {
        if (!first) {
            if (seperator != -1) {
                fputc(seperator, stdout);
            }
        }
        first = false;
        char *path = realpath(argv[optind], 0);
        if (path) {
            fprintf(stdout, "%s", path);
            free(path);
        } else {
            perror("realpath");
            exit_status++;
        }
        optind++;
    }
    if (seperator != -1) {
        fputc(seperator, stdout);
    }
    return exit_status;
}
