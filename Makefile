# Toplevel Makefile for UPX

SHELL = /bin/sh

srcdir = .
top_srcdir = .


default:
	@echo "UPX info: please choose a target for 'make'"

all mostlyclean clean distclean maintainer-clean:
	$(MAKE) -C src/stub/util/sstrip $@
	$(MAKE) -C src/stub $@
	$(MAKE) -C src $@
	$(MAKE) -C doc $@

dist: distclean
	false

cvs-admin-ko:
	cvs admin -ko .
	cvs update -P -d


# automatically generate ChangeLog from CVS
ChangeLog: ChangeLog.cvs

ChangeLog.cvs:
	perl $(srcdir)/scripts/cvs2cl.pl --utc -f ChangeLog.cvs

ChangeLog.cvsps:
	cvsps > $@


.PHONY: all mostlyclean clean distclean maintainer-clean
.PHONY: dist cvs-admin-ko ChangeLog

