# Better ERRor

A c library that help with errors

## Table of Contents

- [Install](#install)
- [Usage](#usage)
- [Example](#example)
- [Maintainers](#maintainers)
- [Contributing](#contributing)
- [License](#license)

## Install

compile and run the test program:
```cmd
make run

```
or

```cmd
gcc -o test.c test.exe -I src/include
```

the library is in the folder [src/include/better_error](src/include/better_error)

## Usage

I try to comment stuff in the [example](#example) and in the library to make it easier

```c
#define BERR_IMPL // stb style library
#include "better_error/berr.h"

bool do_div(int argc, char **argv) {
    if (argc != 3) {
        berr_set_error_file("invalid number of args: %d instead of 3", argc);  // set an error including file info
        return false;
    }
    // ...

    return true;
}

int main(int argc, char **argv) {
    if (!berr_init()) { return 1; }

    // set a windows error 
    SetLastError(ERROR_FILE_NOT_FOUND);
    // query an windows error
    if (!berr_query_error_windows(GetLastError())) { return 1; }
    printf("error: %s\n", berr_get_error());  // get error

    if (!do_div(argc, argv)) {
        printf("error: %s\n", berr_get_error());  // get error
        return 1;
    }

    if (!berr_quit()) { return 1; }
    return 0;
}
```

## Example

A better example in the file [test.c](test.c)

## Maintainers

[@raiseFlaymeException](https://github.com/raiseFlaymeException).

## Contributing

Feel free to contibute [Open an issue](https://github.com/raiseFlaymeException/better_error/issues/new) or submit PRs.

## License

[ZLIB](LICENSE) © raiseFlaymeException
