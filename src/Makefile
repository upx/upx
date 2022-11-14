#
# UPX Makefile - needs GNU make
#

# IMPORTANT NOTE: this Makefile is deprecated - please
#   directly use the CMake build instead!

MAKEFLAGS += -r
.SUFFIXES:
export SHELL = /bin/sh

ifndef srcdir
  srcdir := $(dir $(lastword $(MAKEFILE_LIST)))
  srcdir := $(shell echo '$(srcdir)' | sed 's,/*$$,,' || echo 'ERROR')
endif
ifndef top_srcdir
  top_srcdir := $(srcdir)/..
endif

#
# redirect to top-level CMake build
#

.DEFAULT_GOAL = build/debug

build/debug:   $(top_srcdir)/build/debug/upx
build/release: $(top_srcdir)/build/release/upx
# shortcuts
all: build/debug build/release
debug: build/debug
release: build/release

$(top_srcdir)/build/debug/upx: PHONY
	$(MAKE) -C $(top_srcdir) build/debug
$(top_srcdir)/build/release/upx: PHONY
	$(MAKE) -C $(top_srcdir) build/release

.PHONY: PHONY

#
# "make run-testsuite"
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
# The expected (old) checksums are in $(top_srcdir)/.github/travis_testsuite_1-expected_sha256sums.sh
# The   actual (new) checksums are in tmp-testsuite/testsuite_1/.sha256sums.recreate
ifneq ($(wildcard $(upx_testsuite_SRCDIR)/files/packed/.),)
ifneq ($(wildcard $(top_srcdir)/.github/travis_testsuite_1.sh),)
run-testsuite: export upx_exe                := $(top_srcdir)/build/release/upx
run-testsuite: export upx_testsuite_SRCDIR   := $(upx_testsuite_SRCDIR)
run-testsuite: export upx_testsuite_BUILDDIR := ./tmp-testsuite
run-testsuite: $(top_srcdir)/build/release/upx PHONY
	time -p bash $(top_srcdir)/.github/travis_testsuite_1.sh
endif
endif

#
# "make clang-format"
#

# automatically format some C++ source code files
ifeq ($(shell uname),Linux)
CLANG_FORMAT_FILES += bele.h bele_policy.h
CLANG_FORMAT_FILES += compress_lzma.cpp compress_ucl.cpp compress_zlib.cpp
CLANG_FORMAT_FILES += except.cpp except.h
CLANG_FORMAT_FILES += file.cpp file.h
CLANG_FORMAT_FILES += linker.cpp linker.h packhead.cpp packmast.cpp packmast.h
CLANG_FORMAT_FILES += main.cpp options.cpp options.h packer.cpp packer.h
CLANG_FORMAT_FILES += p_tos.cpp p_tos.h
CLANG_FORMAT_FILES += s_djgpp2.cpp s_object.cpp s_vcsa.cpp s_win32.cpp screen.h
CLANG_FORMAT_FILES += ui.cpp ui.h work.cpp
CLANG_FORMAT_FILES += $(wildcard util/[a-ln-z]*.[ch]* util/mem*.[ch]*)
clang-format: PHONY
	$(top_srcdir)/misc/scripts/upx-clang-format -i $(addprefix $(top_srcdir)/src/,$(sort $(CLANG_FORMAT_FILES)))
endif

#
# "make check-whitespace"
#

CHECK_WHITESPACE =
ifeq ($(shell uname),Linux)
CHECK_WHITESPACE = $(top_srcdir)/misc/scripts/check_whitespace.sh $(top_srcdir)
ifneq ($(wildcard $(top_srcdir)/.git/.),)
CHECK_WHITESPACE = $(top_srcdir)/misc/scripts/check_whitespace_git.sh $(top_srcdir)
endif
check-whitespace : PHONY ; $(CHECK_WHITESPACE)
endif

# vim:set ts=8 sw=8 noet:
