LIBS    = x11 libconfig
CFLAGS  += $(shell pkg-config --cflags $(LIBS)) -std=c99 -Wall -Wextra -pedantic
LDFLAGS += $(shell pkg-config --libs $(LIBS))

PROGRAM = xob
MANPAGE = doc/xob.1
SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)

INSTALL         ?= install
INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA    ?= $(INSTALL) -m 644
prefix          ?= /usr/local
bindir          ?= $(prefix)/bin
datarootdir     ?= $(prefix)/share
mandir          ?= $(datarootdir)/man
man1dir         ?= $(mandir)/man1

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

install: $(PROGRAM) $(MANPAGE)
	mkdir --parents "$(DESTDIR)$(bindir)"
	$(INSTALL_PROGRAM) "$(PROGRAM)" -t "$(DESTDIR)$(bindir)"
	mkdir --parents "$(DESTDIR)$(man1dir)"
	$(INSTALL_DATA) "$(MANPAGE)" -t "$(DESTDIR)$(man1dir)"

uninstall:
	rm -f "$(DESTDIR)$(bindir)/$(PROGRAM)"
	rm -f "$(DESTDIR)$(man1dir)/$(MANPAGE)"

clean:
	rm -f $(OBJECTS)
	rm -f $(PROGRAM)

src/conf.o: src/conf.h
src/display.o: src/display.h src/conf.h
src/main.o: src/main.h src/display.h src/conf.h

.PHONY: all install uninstall clean

