# Toplevel Makefile for UPX

all:
	$(MAKE) -C src/stub
	$(MAKE) -C src
	$(MAKE) -C doc

mostlyclean clean distclean maintainer-clean:
	$(MAKE) -C src/stub $@
	$(MAKE) -C src $@
	$(MAKE) -C doc $@

dist: distclean
	false

zip:
	cd .. && rm -f upx-currenz.zip && zip -r upx-current.zip upx


cvs-admin-ko:
	cvs admin -ko .
	cvs update

ChangeLog:
	perl scripts/cvs2cl.pl --utc -f ChangeLog.cvs

.PHONY: all mostlyclean clean distclean maintainer-clean
.PHONY: dist cvs-admin-ko ChangeLog

.NOEXPORT:

