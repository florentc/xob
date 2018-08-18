LIBS    = x11 libconfig
CFLAGS  = `pkg-config --cflags $(LIBS)` -std=c99 -Wall -Wextra -pedantic
LDFLAGS = `pkg-config --libs $(LIBS)`

PROGRAM = xob
MANPAGE = xob.1
SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)

INSTALL_PROGRAM = install
INSTALL_DATA    = install --mode=644
PREFIX         ?= /usr
BINPREFIX      ?= $(PREFIX)/bin
MANPREFIX      ?= $(PREFIX)/share/man

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

install: $(PROGRAM) $(MANPAGE)
	mkdir --parents "$(DESTDIR)$(BINPREFIX)"
	$(INSTALL_PROGRAM) "$(PROGRAM)" -t "$(DESTDIR)$(BINPREFIX)"
	mkdir --parents "$(DESTDIR)$(MANPREFIX)/man1"
	$(INSTALL_DATA) "doc/$(MANPAGE)" -t "$(DESTDIR)$(MANPREFIX)/man1"

uninstall:
	rm -f "$(DESTDIR)$(BINPREFIX)/$(PROGRAM)"
	rm -f "$(DESTDIR)$(MANPREFIX)/man1/$(MANPAGE)"

clean:
	rm -f $(OBJECTS)
	rm -f $(PROGRAM)

src/conf.o: src/conf.h
src/display.o: src/display.h src/conf.h
src/main.o: src/main.h src/display.h src/conf.h

.PHONY: all install uninstall clean

