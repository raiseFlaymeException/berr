CC = gcc
CXX = g++
DEBUG = gdb

STANDARD = -std=c11

OPTI = -O3

WARNINGS = -Wall -Wextra -Wpedantic -Werror

HEADERDIR = src/include
LIBDIR = src/lib

LIBC_CPP = -static-libstdc++ -static-libgcc
HIDEWINDOW = -mwindows

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

HEADERS = $(call rwildcard,src/include,*.h)
EXTERNAL_HEADERS = $(call rwildcard,src/include/external,*.h)
INTERNAL_HEADERS = $(filter-out $(EXTERNAL_HEADERS),$(HEADERS))
SOURCES = $(call rwildcard,src/c,*.c)
COMPILED = $(subst .c,.o,$(SOURCES)) 

QUIET = > nul 2>&1

LIB =

all: bin/test.exe

bin/test.exe: test.o $(COMPILED)
	$(CC) test.o $(COMPILED) -o bin/test.exe $(LIB) -I $(HEADERDIR) -L $(LIBDIR) $(WARNING) $(STANDARD)

.PHONY: test_debug
test_debug: test.c $(SOURCES)
	$(CC) -ggdb3 test.c $(SOURCES) -o bin/test_debug.exe $(LIB) -I $(HEADERDIR) -L $(LIBDIR) $(WARNING) $(STANDARD)

.PHONY: test_preprocess
test_preprocess: test.c $(COMPILED) $(SOURCES)
	$(CC) -E $(SOURCES) $(COMPILED) $(LIB) -I $(HEADERDIR) -L $(LIBDIR) $(WARNING) $(STANDARD) $(OPTI) > bin/test.ipp

.PHONY: debug
debug: test_debug
	$(DEBUG) bin/test_debug.exe

.PHONY: release_bin
release_bin: test.c $(SOURCES)
	$(CC) test.c $(SOURCES) -o bin/test_release.exe $(LIB) $(HIDEWINDOW) $(LIBC_CPP) -I $(HEADERDIR) -L $(LIBDIR) -D RELEASE $(WARNING) $(STANDARD) $(OPTI)

.PHONY: run
run: bin/test.exe
	./bin/test.exe

.PHONY: rebuild
rebuild: clean bin/test.exe

.PHONY: rerun
rerun: clean run

.PHONY: clang-tidy
clang-tidy: $(INTERNAL_HEADERS) $(SOURCES)
	clang-tidy $(INTERNAL_HEADERS) $(SOURCES)

code-action: $(INTERNAL_HEADERS) $(SOURCES)
	clang-tidy $(INTERNAL_HEADERS) $(SOURCES) -fix -fix-errors

format: $(INTERNAL_HEADERS) $(SOURCES)
	clang-format $(INTERNAL_HEADERS) $(SOURCES) -i

test.o: test.c
	$(CC) -c test.c -o test.o -I $(HEADERDIR) $(WARNING) $(STANDARD) $(OPTI)

src/c/%.o: src/c/%.c
	$(CC) -c $< -o $@ -I $(HEADERDIR) $(WARNINGS) $(STANDARD) $(OPTI)

.PHONY: clean
.SILENT: clean
clean:
	-del test.o $(QUIET)
	-del /S src\c\*.o $(QUIET)

.PHONY: cleanmore
.SILENT: cleanmore
cleanmore: clean
	-del bin\*.exe $(QUIET)

	-del bin\test.ipp $(QUIET)
