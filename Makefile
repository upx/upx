#
# UPX toplevel Makefile - needs GNU make 3.81 or better
#
# Copyright (C) 1996-2020 Markus Franz Xaver Johannes Oberhumer
#

MAKEFLAGS += -rR
.SUFFIXES:
export SHELL = /bin/sh

srcdir = .
top_srcdir = .
include $(wildcard $(top_srcdir)/Makevars.global ./Makevars.local)

# info: src/stub needs special build tools from https://github.com/upx/upx-stubtools
BUILD_STUB = 0
ifneq ($(wildcard $(HOME)/local/bin/bin-upx/upx-stubtools-check-version),)
BUILD_STUB = 1
endif
ifneq ($(wildcard $(HOME)/bin/bin-upx/upx-stubtools-check-version),)
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

.PHONY: default all mostlyclean clean distclean maintainer-clean

# vim:set ts=8 sw=8 noet:
