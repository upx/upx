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

cvs-admin-ko:
	cvs admin -ko .
	cvs update

.PHONY: all clean distclean dist cvs-admin-ko

.NOEXPORT:

