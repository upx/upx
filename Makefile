# Toplevel Makefile for UPX

all:
	$(MAKE) -C src/stub
	$(MAKE) -C src
	$(MAKE) -C doc

clean:
	$(MAKE) -C src/stub $@
	$(MAKE) -C src $@
	$(MAKE) -C doc $@

distclean: clean

dist:
	sh ./maint/util/laszlo.sh

.PHONY: all clean distclean dist

.NOEXPORT:

