# Toplevel Makefile for UPX

MAKEFLAGS += -rR
.SUFFIXES:
export SHELL = /bin/sh

srcdir = .
top_srcdir = .


default:
	@echo "UPX info: please choose a target for 'make'"

all mostlyclean clean distclean maintainer-clean:
ifneq ($(wildcard $(HOME)/local/bin/bin-upx),)
	# these need special build tools
	$(MAKE) -C src/stub/util/sstrip $@
	$(MAKE) -C src/stub $@
endif
	$(MAKE) -C src $@
	$(MAKE) -C doc $@

dist: distclean
	false

# automatically generate ChangeLog from Mercurial repo
ChangeLog:
ifneq ($(wildcard .hg/data),)
	hg log --style=changelog > $@
else
	@echo "UPX info: no hg repo found"
endif


.PHONY: default all mostlyclean clean distclean maintainer-clean
.PHONY: dist ChangeLog

