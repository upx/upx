# Toplevel Makefile for UPX

all:
	$(MAKE) -C src/stub
	$(MAKE) -C src
	$(MAKE) -C doc

clean distclean:
	$(MAKE) -C src/stub $@
	$(MAKE) -C src $@
	$(MAKE) -C doc $@

dist: distclean
	false

cvs-admin-ko:
	cvs admin -ko .
	cvs update

ChangeLog:
	perl scripts/cvs2cl.pl --utc -f ChangeLog.cvs

.PHONY: all clean distclean dist cvs-admin-ko ChangeLog

.NOEXPORT:

