LIBS    = x11 libconfig xrender
CFLAGS  += $(shell pkg-config --cflags $(LIBS)) -std=c99 -Wall -Wextra -pedantic
LDFLAGS += $(shell pkg-config --libs $(LIBS))

PROGRAM = xob
MANPAGE = doc/xob.1
SYSCONF = styles.cfg
SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)

INSTALL         ?= install
INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA    ?= $(INSTALL) -m 644
prefix          ?= /usr/local
bindir          ?= $(prefix)/bin
sysconfdir      ?= $(prefix)/etc
datarootdir     ?= $(prefix)/share
mandir          ?= $(datarootdir)/man
man1dir         ?= $(mandir)/man1

ifdef enable_alpha
	CFLAGS += -DALPHA=1
endif

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.c %.h
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
	rm -f $(OBJECTS)
	rm -f $(PROGRAM)

src/conf.o: src/conf.h
src/display.o: src/display.h src/conf.h
src/main.o: src/main.h src/display.h src/conf.h

.PHONY: all install uninstall clean
