# wikid version
VERSION = 0.1.1

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# flags
CPPFLAGS = -D_DEFAULT_SOURCE\
           -D_BSD_SOURCE\
           -D_POSIX_C_SOURCE=2\
           -DVERSION=\"${VERSION}\"

CFLAGS   = -std=c99\
           -Wall\
           -Wno-deprecated-declarations\
           -Os\
           -pedantic\
           ${CPPFLAGS}

LDFLAGS  = -lcurl\
           -lncurses

# compiler and linker
CC = cc
