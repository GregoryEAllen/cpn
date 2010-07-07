
#include <unistd.h>

unsigned NumProcessorsOnline() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}
