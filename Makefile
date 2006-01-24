# Toplevel Makefile for UPX

MAKEFLAGS += -rR
.SUFFIXES:
export SHELL = /bin/sh

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


ifneq ($(wildcard CVS/R*),)
CVS_ROOT := $(shell cat CVS/Root)
CVS_REPO := $(shell cat CVS/Repository)
endif

cvs-info:
	@echo 'CVS info: $(CVS_ROOT) $(CVS_REPO)'

cvs-admin-ko:
	cvs admin -ko .
	cvs -q -z6 update -P -d

# automatically generate ChangeLog from CVS
ChangeLog ChangeLog.cvs:
	perl $(srcdir)/scripts/cvs2cl.pl --utc -f ChangeLog.cvs


.PHONY: all mostlyclean clean distclean maintainer-clean
.PHONY: dist cvs-info cvs-admin-ko ChangeLog ChangeLog.cvs

