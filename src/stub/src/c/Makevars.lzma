# UPX unconditionally uses its own version in vendor/lzma-sdk because
# that version works fine since 2006 and that is the only version
# that is actually sufficiently tested!!!

ifeq ($(wildcard $(top_srcdir)/vendor/lzma-sdk/C/.),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif

override UPX_LZMADIR := $(top_srcdir)/vendor/lzma-sdk
override UPX_LZMA_VERSION := 0x443
