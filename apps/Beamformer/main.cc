
#include <stdio.h>
#include <string.h>
#include <libgen.h>


int hb_main(int argc, char **argv);

int vb_main(int argc, char **argv);

int ompbf_main(int argc, char **argv);

int cpnbf_main(int argc, char **argv);

int main(int argc, char **argv) {
    char *selfname = basename(*argv);
    if (*selfname == 'v') {
        return vb_main(argc, argv);
    } else if (*selfname == 'h') {
        return hb_main(argc, argv);
    } else if (strcmp(selfname, "ompbf") == 0) {
        return ompbf_main(argc, argv);
    } else if (strcmp(selfname, "cpnbf") == 0) {
        return cpnbf_main(argc, argv);
    } else {
        fprintf(stderr, "Unknown\n");
    }
    return 1;
}
