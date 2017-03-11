CC = gcc
CFLAGS = -std=c99 -O2 -Wall -finline-functions -Iinclude
LIBRARY_FLAGS = -Llib -lsdd -lm
EXEC_FILE = sdd

SDD_PACKAGE = \"sdd\"
SDD_VERSION = \"1.1.1\"
SDD_DATE    = \"January\ 31,\ 2014\"
SDD_VERSION_FLAGS = -DSDD_PACKAGE=${SDD_PACKAGE} -DSDD_VERSION=${SDD_VERSION} -DSDD_DATE=${SDD_DATE}

COMPILER_SRC = src/main.c \
	src/fnf/compiler-auto.c src/fnf/compiler-manual.c src/fnf/utils.c src/fnf/fnf.c \
	src/search/search.c src/search/state.c

COMPILER_OBJS=$(COMPILER_SRC:.c=.o) src/getopt.o

HEADERS = sddapi.h compiler.h parameters.h

sdd: $(COMPILER_OBJS)
	$(CC) $(CFLAGS) $(COMPILER_OBJS) $(LIBRARY_FLAGS) -o $(EXEC_FILE)

src/getopt.o: src/getopt.c
	$(CC) $(SDD_VERSION_FLAGS) $(CFLAGS) -c $< -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(COMPILER_OBJS) $(EXEC_FILE)
