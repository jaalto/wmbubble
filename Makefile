# bubblemon configuration
EXTRA = -DENABLE_DUCK
EXTRA += -DENABLE_CPU
EXTRA += -DENABLE_MEMSCREEN
EXTRA += -DKDE_DOCKAPP
EXTRA += -DUPSIDE_DOWN_DUCK

# where to install this program
PREFIX = /usr/local

# no user serviceable parts below
EXTRA += $(WMAN)
# optimization cflags
USER_CFLAGS = -O3 -ansi -Wall
# profiling cflags
# USER_CFLAGS = -ansi -Wall -pg -O3 -DPRO
# test coverage cflags
# USER_CFLAGS = -fprofile-arcs -ftest-coverage -Wall -ansi -g -DPRO
BUILD_CFLAGS = `gtk-config --cflags`
CFLAGS = $(USER_CFLAGS) $(BUILD_CFLAGS) ${EXTRA}

BINARY=bubblemon
SHELL=sh
OS = $(shell uname -s)
OBJS = bubblemon.o
CC = gcc

# special things for Linux
ifeq ($(OS), Linux)
    OBJS += sys_linux.o
    LIBS = `gtk-config --libs | sed "s/-lgtk//g"`
    INSTALL = -m 755
endif

# special things for FreeBSD
ifeq ($(OS), FreeBSD)
    OBJS += sys_freebsd.o
    LIBS = `gtk-config --libs | sed "s/-lgtk//g"` -lkvm
    INSTALL = -c -g kmem -m 2755 -o root
endif

# special things for OpenBSD
ifeq ($(OS), OpenBSD)
    OBJS += sys_openbsd.o
    LIBS = `gtk-config --libs | sed "s/-lgtk//g"`
endif

#special things for SunOS
ifeq ($(OS), SunOS)

    # try to detect if gcc is available (also works if you call gmake CC=cc to
    # select the sun compilers on a system with both)
    COMPILER=$(shell \
        if [ `$(CC) -v 2>&1 | egrep -c '(gcc|egcs|g\+\+)'` = 0 ]; then \
	    echo suncc; else echo gcc; fi)

    # if not, fix up CC and the CFLAGS for the Sun compiler
    ifeq ($(COMPILER), suncc)
	CC=cc
	USER_CFLAGS=-v -xO3
    endif

    ifeq ($(COMPILER), gcc)
	USER_CFLAGS=-O3 -Wall
    endif
    CFLAGS = $(USER_CFLAGS) $(BUILD_CFLAGS) ${EXTRA}
    OBJS += sys_sunos.o
    LIBS = `gtk-config --libs` -lkstat -lm
    INSTALL = -m 755
endif

CFLAGS += -DNAME=\"$(BINARY)\"

all: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) $(CFLAGS) -o $(BINARY) $(OBJS) $(LIBS)

clean:
	rm -f $(BINARY) *.o *.bb* *.gcov gmon.* *.da *~

install:
	install $(INSTALL) $(BINARY) $(PREFIX)/bin
