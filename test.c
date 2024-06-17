#define BERR_IMPL
#include "better_error/berr.h"

#include <stdio.h>
#include <string.h>

// #define TEST_WINDOWS

// a stupid function that may failed
bool do_div(int argc, char **argv) {
    if (argc != 3) {
        berr_set_error_file("invalid number of args: %d instead of 3", argc);
        return false;
    }
    char  *res;
    double n1 = strtod(argv[1], &res);
    if (argv[1] + strlen(argv[1]) != res) {
        berr_set_error_file("invalid argument 1: %s", argv[1]);
        return false;
    }
    double n2 = strtod(argv[2], &res);
    if (argv[2] + strlen(argv[2]) != res) {
        berr_set_error_file("invalid argument 2: %s", argv[2]);
        return false;
    }
    if (n2 == 0) {
        berr_set_error_file("argument 2 must be non zero");
        return false;
    }

    printf("result: %lf\n", n1 / n2);
    return true;
}

int main(int argc, char **argv) {
    if (!berr_init()) { return 1; }

#ifdef TEST_WINDOWS
    // set an error to see if this works:
    SetLastError(ERROR_FILE_NOT_FOUND);
    if (!berr_query_error_windows(GetLastError())) { return 1; }
    printf("error: %s\n", berr_get_error());
    return 1;
#endif

    if (!do_div(argc, argv)) {
        printf("error: %s\n", berr_get_error());
        return 1;
    }

    if (!berr_quit()) { return 1; }
    return 0;
}
