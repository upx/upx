#
# UPX src Makefile - needs GNU make and CMake >= 3.13
# Copyright (C) Markus Franz Xaver Johannes Oberhumer
#

# NOTE: this Makefile is deprecated - please directly use
# the top-level CMake build instead.

ifndef srcdir
  srcdir := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
endif
ifndef top_srcdir
  top_srcdir := $(srcdir)/..
endif

#***********************************************************************
# redirect to top-level CMake build
#***********************************************************************

# NOTE that the top-level Makefile .DEFAULT_GOAL is build/release
.DEFAULT_GOAL = build/all

.NOTPARALLEL: # because the actual builds use "cmake --parallel"
.PHONY: PHONY
.SECONDEXPANSION:
.SUFFIXES:

build/debug build/release build/all:                PHONY; $(MAKE) -C $(top_srcdir) $@
build/debug+test build/release+test build/all+test: PHONY; $(MAKE) -C $(top_srcdir) $@
# shortcuts
debug release all:                                  PHONY; $(MAKE) -C $(top_srcdir) $@
debug+test release+test all+test:                   PHONY; $(MAKE) -C $(top_srcdir) $@

test:: build/all+test PHONY

# OLD target names [deprecated]
$(top_srcdir)/build/debug:   build/debug PHONY
$(top_srcdir)/build/release: build/release PHONY

#***********************************************************************
# make run-testsuite
# git clone https://github.com/upx/upx-testsuite.git
#***********************************************************************

# search for the UPX testsuite; you can override by setting "upx_testsuite_SRCDIR"
ifndef upx_testsuite_SRCDIR
# search standard locations below $(top_srcdir)
ifneq ($(wildcard $(top_srcdir)/../upx--upx-testsuite.git/files/packed/.),)
upx_testsuite_SRCDIR := $(top_srcdir)/../upx--upx-testsuite.git
else ifneq ($(wildcard $(top_srcdir)/../upx-testsuite.git/files/packed/.),)
upx_testsuite_SRCDIR := $(top_srcdir)/../upx-testsuite.git
else ifneq ($(wildcard $(top_srcdir)/../upx-testsuite/files/packed/.),)
upx_testsuite_SRCDIR := $(top_srcdir)/../upx-testsuite
endif
endif

# run the UPX testsuite
# The expected (old) checksums are in $(top_srcdir)/misc/testsuite/upx_testsuite_1-expected_sha256sums.sh
# The   actual (new) checksums are in ./tmp-upx-testsuite-*/testsuite_1/.sha256sums.recreate
ifneq ($(wildcard $(upx_testsuite_SRCDIR)/files/packed/.),)
ifneq ($(wildcard $(top_srcdir)/misc/testsuite/upx_testsuite_1.sh),)
run-testsuite: run-testsuite-release PHONY
run-testsuite-%: export upx_testsuite_SRCDIR := $(upx_testsuite_SRCDIR)
run-testsuite-debug: export upx_testsuite_BUILDDIR := ./tmp-upx-testsuite-debug
run-testsuite-debug: export upx_exe := $(top_srcdir)/build/debug/upx
run-testsuite-debug: build/debug PHONY
	time -p bash $(top_srcdir)/misc/testsuite/upx_testsuite_1.sh
run-testsuite-release: export upx_testsuite_BUILDDIR := ./tmp-upx-testsuite-release
run-testsuite-release: export upx_exe := $(top_srcdir)/build/release/upx
run-testsuite-release: build/release PHONY
	time -p bash $(top_srcdir)/misc/testsuite/upx_testsuite_1.sh
endif
endif

#***********************************************************************
# make check-whitespace
#***********************************************************************

ifneq ($(wildcard /usr/bin/env),) # need Unix utils like bash, perl, sed, xargs, etc.

CHECK_WHITESPACE = bash $(top_srcdir)/misc/scripts/check_whitespace.sh $(top_srcdir)
ifneq ($(wildcard $(top_srcdir)/.git/.),)
CHECK_WHITESPACE = bash $(top_srcdir)/misc/scripts/check_whitespace_git.sh $(top_srcdir)
endif
check-whitespace: PHONY; $(CHECK_WHITESPACE)

endif # /usr/bin/env

#***********************************************************************
# make clang-format
#***********************************************************************

# automatically format some C++ source code files
ifneq ($(wildcard /usr/bin/env),)
ifeq ($(shell uname),Linux)

# Markus loves clang-format, but John hates it; find a compromise
CLANG_FORMAT_EXCLUDE_FILES += stub/%.h util/miniacc.h
CLANG_FORMAT_EXCLUDE_FILES += p_lx_% p_mach% p_unix% p_vmlin%
CLANG_FORMAT_FILES := $(sort $(wildcard *.[ch]* ../maint/src/*.[ch]* */*.[ch]*))
CLANG_FORMAT_FILES += $(sort $(wildcard stub/tools/*/*.[ch]*))
CLANG_FORMAT_FILES += $(sort $(wildcard ../misc/cmake/try_compile/*.[ch]*))
CLANG_FORMAT_FILES := $(sort $(filter-out $(CLANG_FORMAT_EXCLUDE_FILES),$(CLANG_FORMAT_FILES)))
clang-format: $(CLANG_FORMAT_FILES) PHONY
	@echo "running upx-clang-format"
	@bash $(top_srcdir)/misc/scripts/upx-clang-format.sh -i $(CLANG_FORMAT_FILES)

endif # Linux
endif # /usr/bin/env

# vim:set ts=8 sw=8 noet:
