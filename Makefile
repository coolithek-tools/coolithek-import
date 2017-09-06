###
###  Makefile mv2mariadb
###

## (optional) private definitions for DEBUG, EXTRA_CFLAGS, DESTDIR
-include priv-settings.mk

DEBUG        ?= 0
EXTRA_CFLAGS ?=
DESTDIR      ?= /usr/local

CC	 = g++
STRIP	 = strip

INCLUDES = -I/usr/include/mariadb
ifeq ($(DEBUG), 1)
CFLAGS   = $(INCLUDES) -Wall -W -Wshadow -Werror -Wl,-O0 -pipe -g -ggdb3 -fno-strict-aliasing
else
CFLAGS   = $(INCLUDES) -Wall -W -Wshadow -Werror -Wl,-O3 -pipe -fno-strict-aliasing
endif
CFLAGS   += -fmax-errors=10
CFLAGS   += $(EXTRA_CFLAGS)

LIBS     = -lstdc++ -ljsoncpp -lmariadb -llzma -lcurl -lpthread
LDFLAGS  = $(LIBS)

ifeq ($(DEBUG), 1)
all: clean mv2mariadb
else
all: clean mv2mariadb strip
endif

PROG_SOURCES = \
	src/mv2mariadb.cpp \
	src/sql.cpp \
	src/helpers.cpp \
	src/filehelpers.cpp \
	src/configfile.cpp \
	src/lzma_dec.cpp \
	src/curl.cpp

mv2mariadb: $(PROG_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(PROG_SOURCES) -o mv2mariadb

install:
	install -m 755 -D mv2mariadb $(DESTDIR)/bin/mv2mariadb

clean:
	rm -f mv2mariadb

strip:
	$(STRIP) mv2mariadb
