#
# UPX toplevel Makefile - needs GNU make 3.81 or better
#

MAKEFLAGS += -rR
.SUFFIXES:
export SHELL = /bin/sh

ifneq ($(findstring $(firstword $(MAKE_VERSION)),3.77 3.78 3.78.1 3.79 3.79.1 3.80),)
$(error GNU make 3.81 or better is required)
endif

srcdir = .
top_srcdir = .
include $(wildcard $(top_srcdir)/Makevars.global ./Makevars.local)


# info: src/stub needs special build tools
BUILD_STUB = 0
ifneq ($(wildcard $(HOME)/local/bin/bin-upx/.),)
BUILD_STUB = 1
endif
ifneq ($(wildcard $(HOME)/bin/bin-upx/.),)
BUILD_STUB = 1
endif


default:
	@echo "UPX info: please choose a target for 'make'"

all mostlyclean clean distclean maintainer-clean:
ifeq ($(BUILD_STUB),1)
	$(MAKE) -C src/stub $@
endif
	$(MAKE) -C src $@
	$(MAKE) -C doc $@


ifneq ($(wildcard .hg/.),)
# automatically generate ChangeLog from local Mercurial repo
ChangeLog:
	hg log --style=changelog > $@
.PHONY: ChangeLog
endif


.PHONY: default all mostlyclean clean distclean maintainer-clean

