###
###  Makefile mv2mysql
###

DEBUG = 1

CC	 = g++
STRIP	 = strip

INCLUDES =
ifeq ($(DEBUG), 1)
CFLAGS   = $(INCLUDES) -Wall -W -Wshadow -Werror -Wl,-O1 -pipe -g -ggdb3 -fno-strict-aliasing
else
CFLAGS   = $(INCLUDES) -Wall -W -Wshadow -Werror -Wl,-O3 -pipe -fno-strict-aliasing
endif
CFLAGS   += -fmax-errors=10

LIBS     = -lstdc++ -ljsoncpp -lmysqlcppconn
LDFLAGS  = $(LIBS)
DESTDIR  = /usr

ifeq ($(DEBUG), 1)
all: clean mv2mysql
else
all: clean mv2mysql strip
endif

PROG_SOURCES = \
	src/mv2mysql.cpp \
	src/helpers.cpp

mv2mysql: $(PROG_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(PROG_SOURCES) -o mv2mysql

install:
	install -m 755 -D mv2mysql $(DESTDIR)/bin/mv2mysql

clean:
	rm -f mv2mysql

strip:
	$(STRIP) mv2mysql
