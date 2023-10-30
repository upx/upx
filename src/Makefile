#
# UPX src Makefile - needs GNU make and CMake >= 3.13
# Copyright (C) Markus Franz Xaver Johannes Oberhumer
#

# NOTE: this Makefile is deprecated - please directly use the CMake build
# instead. And see the top-level Makefile for some pre-defined CMake
# build configurations.

ifndef srcdir
  srcdir := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
endif
ifndef top_srcdir
  top_srcdir := $(srcdir)/..
endif

#
# redirect to top-level CMake build
#

# NOTE that top-level Makefile .DEFAULT_GOAL is build/release
.DEFAULT_GOAL = build/debug

build/debug:   $(top_srcdir)/build/debug/upx
build/release: $(top_srcdir)/build/release/upx

.NOTPARALLEL: # because the actual builds use "cmake --parallel"
.PHONY: PHONY
.SECONDEXPANSION:
.SUFFIXES:

# shortcuts (all => debug + release)
debug:   build/debug
release: build/release
all build/all: build/debug build/release

$(top_srcdir)/build/debug/upx: PHONY
	$(MAKE) -C $(top_srcdir) build/debug
$(top_srcdir)/build/release/upx: PHONY
	$(MAKE) -C $(top_srcdir) build/release

#
# "make run-testsuite"
# see https://github.com/upx/upx-testsuite.git
#

# search for the UPX testsuite -- git clone https://github.com/upx/upx-testsuite.git
# you also can override upx_testsuite_SRCDIR
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
# The   actual (new) checksums are in ./tmp-upx-testsuite/testsuite_1/.sha256sums.recreate
ifneq ($(wildcard $(upx_testsuite_SRCDIR)/files/packed/.),)
ifneq ($(wildcard $(top_srcdir)/misc/testsuite/upx_testsuite_1.sh),)
run-testsuite: run-testsuite-release
run-testsuite-%: export upx_testsuite_SRCDIR   := $(upx_testsuite_SRCDIR)
run-testsuite-%: export upx_testsuite_BUILDDIR := ./tmp-upx-testsuite
run-testsuite-debug: export upx_exe := $(top_srcdir)/build/debug/upx
run-testsuite-debug: PHONY $(top_srcdir)/build/debug/upx
	time -p bash $(top_srcdir)/misc/testsuite/upx_testsuite_1.sh
run-testsuite-release: export upx_exe := $(top_srcdir)/build/release/upx
run-testsuite-release: PHONY $(top_srcdir)/build/release/upx
	time -p bash $(top_srcdir)/misc/testsuite/upx_testsuite_1.sh
endif
endif

#
# "make check-whitespace"
#

ifneq ($(wildcard /usr/bin/env),) # needs bash, perl, xargs, etc.
CHECK_WHITESPACE = $(top_srcdir)/misc/scripts/check_whitespace.sh $(top_srcdir)
ifneq ($(wildcard $(top_srcdir)/.git/.),)
CHECK_WHITESPACE = $(top_srcdir)/misc/scripts/check_whitespace_git.sh $(top_srcdir)
endif
check-whitespace: PHONY; $(CHECK_WHITESPACE)
endif

#
# "make clang-format"
#

# automatically format some C++ source code files
ifeq ($(shell uname),Linux)
# Markus loves clang-format, but John hates it; find a compromise
CLANG_FORMAT_EXCLUDE_FILES += miniacc.h stub/%.h
CLANG_FORMAT_EXCLUDE_FILES += p_elf.h p_elf_enum.h p_lx_% p_mach% p_unix% p_vmlin%
CLANG_FORMAT_FILES := $(sort $(wildcard *.[ch]* ../maint/src/*.[ch]* */*.[ch]*))
CLANG_FORMAT_FILES += $(sort $(wildcard stub/tools/*/*.[ch]*))
CLANG_FORMAT_FILES := $(filter-out $(CLANG_FORMAT_EXCLUDE_FILES),$(CLANG_FORMAT_FILES))
clang-format: PHONY $(CLANG_FORMAT_FILES)
	@echo "running upx-clang-format"
	@$(top_srcdir)/misc/scripts/upx-clang-format.sh -i $(CLANG_FORMAT_FILES)
endif

# vim:set ts=8 sw=8 noet:
