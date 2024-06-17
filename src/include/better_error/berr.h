#ifndef BERR_H
#define BERR_H

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// define this macro to use mutex so berr_* works with multiple thread
// #define BERR_USE_MUTEX

// the file format of the print (can be overwritted but must have %s (the file) then %d (the line)
// and then %s (the error message))
#ifndef BERR_PRINT_FILE_FORMAT
#define BERR_PRINT_FILE_FORMAT "[%s:%d]: %s"
#endif // BERR_PRINT_FILE_FORMAT

///
/// @brief initialize berr, must be called before using any berr_* function
///
/// @return true if not failed else false
///
bool berr_init();
///
/// @brief deinitialize berr, must be called once after we don't have to use any berr_* function any
/// more
///
/// @return true if not failed else false
///
bool berr_quit();

///
/// @brief set the error using variadic arguments
///
/// @param[in] error the error (can have special format like printf)
/// @param[in, out] args the arguments to the function, (the variadic list will be invalid after
/// this call)
/// @return true if not failed else false
///
bool berr_vset_error(const char *error, va_list args);
///
/// @brief set the error
///
/// @param[in] error the error (can have special format like printf)
/// @return true if not failed else false
///
bool berr_set_error(const char *error, ...);
///
/// @brief set the error including file and line info (see BERR_PRINT_FILE_FORMAT)
/// WARNING: internal function, call berr_set_error_file instead
///
/// @param[in] file the file name where this function is called
/// @param[in] line the line where this function is called
/// @param[in] error the error (can have special format like printf)
/// @return true if not failed else false
///
bool berr__set_error_file(const char *file, int line, const char *error, ...);

///
/// @brief set the error including file and line info (see BERR_PRINT_FILE_FORMAT)
///
/// @param[in] error the error (can have special format like printf)
/// @return true if not failed else false
///
#define berr_set_error_file(...) berr__set_error_file(__FILE__, __LINE__, __VA_ARGS__)

///
/// @brief get the error
///
/// @return get the last error (can be NULL if no error was set / can still hold the last error)
///
const char *berr_get_error();

#if defined(__WIN32) || defined(__WIN64)
///
/// @brief query the error message in the error
///
/// @param[in] error the error number returned by GetLastError()
/// @return true if not failed otherwise false
///
bool berr_query_error_windows(int error);
///
/// @brief get the error message
/// (see berr_query_error_windows and berr_get_error)
///
/// @param[in] error the error number returned by GetLastError()
/// @return the error if not failed otherwise NULL
///
const char *berr_get_error_windows(int error);
#endif

#endif // BERR_H
#ifdef BERR_IMPL

#if defined(__WIN32) || defined(__WIN64)
#include <windows.h>
#endif

#ifdef BERR_USE_MUTEX
#include "pthread.h"
#endif // BERR_USE_MUTEX

static struct {
    char *msg;
#ifdef BERR_USE_MUTEX
    pthread_mutex_t mut;
#endif // BERR_USE_MUTEX
} berr_error = {0};

static char *vasprintf(const char *format, va_list args) {
    int size = vsnprintf(NULL, 0, format, args) + 1;
    if (size < 0) { return NULL; }

    char *buf = malloc(size);
    if (!buf) { return NULL; }

    int err = vsnprintf(buf, size, format, args);
    return (err < 0 || size < err) ? NULL : buf;
}
static char *asprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    char *result = vasprintf(format, args);

    va_end(args);
    return result;
}

static bool berr_error_free() {
#ifdef BERR_USE_MUTEX
    if (pthread_mutex_lock(&berr_error.mut) != 0) { return false; }
#endif // BERR_USE_MUTEX
    if (berr_error.msg) { free(berr_error.msg); }
#ifdef BERR_USE_MUTEX
    if (pthread_mutex_unlock(&berr_error.mut) != 0) { return false; }
#endif // BERR_USE_MUTEX
    return true;
}
static bool berr_error_set(char *value) {
#ifdef BERR_USE_MUTEX
    if (pthread_mutex_lock(&berr_error.mut) != 0) { return false; }
#endif // BERR_USE_MUTEX
    berr_error.msg = value;
#ifdef BERR_USE_MUTEX
    if (pthread_mutex_unlock(&berr_error.mut) != 0) { return false; }
#endif // BERR_USE_MUTEX
    return true;
}

bool berr_init() {
#ifdef BERR_USE_MUTEX
    if (pthread_mutex_init(&berr_error.mut, NULL) != 0) { return false; }
#endif // BERR_USE_MUTEX
    return true;
}

bool berr_quit() {
    if (!berr_error_free()) { return false; }
#ifdef BERR_USE_MUTEX
    if (pthread_mutex_destroy(&berr_error.mut) != 0) { return false; }
#endif // BERR_USE_MUTEX
    return true;
}

bool berr_vset_error(const char *error, va_list args) {
    // we free first so we can overide without leak
    if (!berr_error_free()) { return false; }
    if (!berr_error_set(vasprintf(error, args))) { return false; }
    return berr_error.msg != NULL;
}
bool berr_set_error(const char *error, ...) {
    va_list args;
    va_start(args, error);

    bool result = berr_vset_error(error, args);

    va_end(args);
    return result;
}
bool berr__set_error_file(const char *file, int line, const char *error, ...) {
    va_list args;
    va_start(args, error);

    char *buf_error = asprintf(BERR_PRINT_FILE_FORMAT, file, line, error);
    if (!buf_error) {
        va_end(args);
        return false;
    }

    bool result = berr_vset_error(buf_error, args);

    va_end(args);
    free(buf_error);
    return result;
}

const char *berr_get_error() { return berr_error.msg; }

#if defined(__WIN32) || defined(__WIN64)
bool berr_query_error_windows(int error) {
    char *res = NULL;
    if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *)&res, 0,
                       NULL) == 0) {
        return false;
    }

    if (!berr_error_free()) { return false; }
    return berr_error_set(res);
}
const char *berr_get_error_windows(int error) {
    if (!berr_query_error_windows(error)) { return NULL; }
    return berr_get_error();
}
#endif
#endif // BERR_IMPL
