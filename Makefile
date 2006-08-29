#
# UPX toplevel Makefile - needs GNU make 3.80 or better
#

MAKEFLAGS += -rR
.SUFFIXES:
export SHELL = /bin/sh

srcdir = .
top_srcdir = .


# info: src/stub needs GNU make 3.81 and special build tools
BUILD_STUB = 0
ifneq ($(wildcard $(HOME)/local/bin/bin-upx/.),)
BUILD_STUB = 1
endif
ifneq ($(wildcard $(HOME)/bin/bin-upx/.),)
BUILD_STUB = 1
endif
ifneq ($(findstring $(firstword $(MAKE_VERSION)),3.79 3.79.1 3.80),)
BUILD_STUB = 0
endif


default:
	@echo "UPX info: please choose a target for 'make'"

all mostlyclean clean distclean maintainer-clean:
ifeq ($(BUILD_STUB),1)
	$(MAKE) -C src/stub $@
endif
	$(MAKE) -C src $@
	$(MAKE) -C doc $@

# automatically generate ChangeLog from Mercurial repo
ChangeLog:
ifneq ($(wildcard .hg/data/.),)
	hg log --style=changelog > $@
else
	@echo "UPX info: no hg repo found"
endif


.PHONY: default all mostlyclean clean distclean maintainer-clean
.PHONY: ChangeLog

