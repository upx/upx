# Toplevel Makefile for UPX

SHELL = /bin/sh

srcdir = .
top_srcdir = .


all mostlyclean clean distclean maintainer-clean:
	$(MAKE) -C src/stub/util/sstrip $@
	$(MAKE) -C src/stub $@
	$(MAKE) -C src $@
	$(MAKE) -C doc $@

dist: distclean
	false

zip:
	cd .. && rm -f upx-current.zip && zip -r upx-current.zip upx

cvs-admin-ko:
	cvs admin -ko .
	cvs update

ChangeLog:
	perl $(srcdir)/scripts/cvs2cl.pl --utc -f ChangeLog.cvs


.PHONY: all mostlyclean clean distclean maintainer-clean
.PHONY: dist cvs-admin-ko ChangeLog

.NOEXPORT:

