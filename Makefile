PREFIX ?= /usr/local
DESTDIR ?=
BINDIR ?= $(PREFIX)/bin

CC ?= cc
CPPFLAGS ?= -D_POSIX_C_SOURCE=200809L
CFLAGS ?= -O2 -Wall -Wextra
LDFLAGS ?=

SRCDIR := src
BINDIR_BUILD := bin
SOURCES := $(wildcard $(SRCDIR)/*.c)
PROGRAMS := $(patsubst $(SRCDIR)/%.c,$(BINDIR_BUILD)/%,$(SOURCES))

.PHONY: all clean install uninstall test

all: $(PROGRAMS)

$(BINDIR_BUILD):
	mkdir -p $@

$(BINDIR_BUILD)/%: $(SRCDIR)/%.c | $(BINDIR_BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -rf $(BINDIR_BUILD)

install: all
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 $(PROGRAMS) $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(addprefix $(DESTDIR)$(BINDIR)/,$(notdir $(PROGRAMS)))

test: all
	sh tests/run.sh
