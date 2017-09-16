###
###  Makefile mv2mariadb
###

PROG_SOURCES = \
	src/$(PROGNAME).cpp \
	src/sql.cpp \
	src/helpers.cpp \
	src/filehelpers.cpp \
	src/configfile.cpp \
	src/lzma_dec.cpp \
	src/curl.cpp \
	src/serverlist.cpp

PROG_OBJS = ${PROG_SOURCES:.cpp=.o}
PROG_DEPS = ${PROG_SOURCES:.cpp=.d}

## (optional) private definitions for DEBUG, EXTRA_CFLAGS etc.
-include priv-settings.mk

DEBUG		?= 0
DESTDIR		?= /usr/local
EXTRA_CFLAGS	?= 
EXTRA_LDFLAGS	?= 
EXTRA_INCLUDES	?= 
EXTRA_LIBS	?= 

PROGNAME = mv2mariadb

C++	 = g++-5
STRIP	 = strip

INCLUDES  = -I/usr/include/mariadb
INCLUDES += $(EXTRA_INCLUDES)

CFLAGS   = $(INCLUDES) -Wall -W -Wshadow -Werror -pipe -fno-strict-aliasing
ifeq ($(DEBUG), 1)
CFLAGS   += -O0 -g -ggdb3
else
CFLAGS   += -O3
endif
CFLAGS   += -std=c++11
CFLAGS   += -fmax-errors=10
CFLAGS   += $(EXTRA_CFLAGS)

LIBS      =
LIBS     += -ljsoncpp
LIBS     += -lmariadb
LIBS     += -llzma
LIBS     += -lcurl
LIBS     += -lpthread
LIBS     += -lexpat
LIBS     += $(EXTRA_LIBS)

LDFLAGS  = $(LIBS)
LDFLAGS  += $(EXTRA_LDFLAGS)

ifeq ($(DEBUG), 1)
all: $(PROGNAME)
else
all: $(PROGNAME) strip
endif

.cpp.o:
	$(C++) $(CFLAGS) -MT $@ -MD -MP -c -o $@ $<

$(PROGNAME): $(PROG_OBJS)
	$(C++) $(LDFLAGS) $(PROG_OBJS) -o $(PROGNAME)

#install:
#	install -m 755 -D $(PROGNAME) $(DESTDIR)/bin/$(PROGNAME)

clean:
	rm -f $(PROGNAME) $(PROG_OBJS) $(PROG_DEPS)

strip:
	$(STRIP) $(PROGNAME)

-include $(PROG_DEPS)
