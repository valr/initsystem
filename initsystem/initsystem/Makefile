
bindir ?= /bin
sysconfdir = /etc # required by init-og and initscripts
libexecdir = /usr/lib/initsystem # required by mkinitramfs
sbindir ?= /sbin
tmpfilesdir ?= /usr/lib/tmpfiles.d

CC ?= gcc
CFLAGS ?= -Wall -static

all: sbin/init-og

sbin/init-og: src/init-og.c
	@mkdir -p sbin
	@$(CC) $(CFLAGS) -o $@ $^

install:
	@mkdir -p         $(DESTDIR)$(bindir)
	@cp -r bin/*      $(DESTDIR)$(bindir)
	@mkdir -p         $(DESTDIR)$(sysconfdir)
	@cp -r etc/*      $(DESTDIR)$(sysconfdir)
	@mkdir -p         $(DESTDIR)$(libexecdir)
	@cp -r libexec/*  $(DESTDIR)$(libexecdir)
	@mkdir -p         $(DESTDIR)$(sbindir)
	@cp -r sbin/*     $(DESTDIR)$(sbindir)
	@mkdir -p         $(DESTDIR)$(tmpfilesdir)
	@cp -r tmpfiles/* $(DESTDIR)$(tmpfilesdir)

uninstall:
	@echo 'Oh crap, there is no uninstall!'

clean:
	@rm -f sbin/init-og initsystem.tar.gz
	@if [ -d sbin ]; then rmdir sbin; fi

dist:
	@tar -caf initsystem.tar.gz LICENSE Makefile archlinux bin etc libexec src tmpfiles

.PHONY: install uninstall clean dist
