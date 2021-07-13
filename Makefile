
PROGRAM = xob
MANPAGE = doc/xob.1
SYSCONF = styles.cfg
LIBS    = x11 libconfig
SOURCES = src/conf.c src/display.c src/main.c

ifdef enable_alpha
	CFLAGS += -DALPHA=1
	LIBS += xrender
	SOURCES += src/display_xrender.c
else
	SOURCES += src/display_xlib.c
endif

OBJECTS = $(SOURCES:.c=.o)
CFLAGS  += $(shell pkg-config --cflags $(LIBS)) -std=c99 -Wall -Wextra -pedantic
LDFLAGS += $(shell pkg-config --libs $(LIBS))

INSTALL         ?= install
INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA    ?= $(INSTALL) -m 644
prefix          ?= /usr/local
bindir          ?= $(prefix)/bin
sysconfdir      ?= $(prefix)/etc
datarootdir     ?= $(prefix)/share
mandir          ?= $(datarootdir)/man
man1dir         ?= $(mandir)/man1


all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -DSYSCONFDIR='"$(sysconfdir)"' -c -o $@ $<

install: $(PROGRAM) $(MANPAGE) $(SYSCONF)
	mkdir --parents "$(DESTDIR)$(bindir)"
	$(INSTALL_PROGRAM) "$(PROGRAM)" -t "$(DESTDIR)$(bindir)"
	mkdir --parents "$(DESTDIR)$(man1dir)"
	$(INSTALL_DATA) "$(MANPAGE)" -t "$(DESTDIR)$(man1dir)"
	mkdir --parents "$(DESTDIR)$(sysconfdir)/$(PROGRAM)"
	$(INSTALL_DATA) "$(SYSCONF)" -t "$(DESTDIR)$(sysconfdir)/$(PROGRAM)"

uninstall:
	rm -f "$(DESTDIR)$(bindir)/$(PROGRAM)"
	rm -f "$(DESTDIR)$(man1dir)/$(MANPAGE)"
	rm -f "$(DESTDIR)$(sysconfdir)/$(PROGRAM)/$(SYSCONF)"
	rmdir "$(DESTDIR)$(sysconfdir)/$(PROGRAM)"

clean:
	rm -f src/*.o
	rm -f $(PROGRAM)

src/conf.o: src/conf.h
src/display.o: src/display.h src/conf.h
src/main.o: src/main.h src/display.h src/conf.h
src/xlib.o: src/display.h
src/xrender.o: src/display.h

.PHONY: all install uninstall clean
