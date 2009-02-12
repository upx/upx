#  Makefile for armpe_tester

MAKEFLAGS += -rR
.SUFFIXES:
.SECONDEXPANSION:
.NOTPARALLEL:
export LC_ALL = C
export SHELL = /bin/sh

# update $PATH for our special stub build tools
ifneq ($(wildcard $(HOME)/local/bin/bin-upx/.),)
export PATH := $(HOME)/local/bin/bin-upx:$(PATH)
endif
ifneq ($(wildcard $(HOME)/bin/bin-upx/.),)
export PATH := $(HOME)/bin/bin-upx:$(PATH)
endif

CFLAGS_W = -Wall -W -Wcast-align -Wcast-qual -Wwrite-strings


all: armpe_tester.out
#all: armpe_tester.exe

armpe_tester.out: armpe_tester.c $(MAKEFILE_LIST)
	arm-linux-gcc-4.1.0 -Wl,--section-start,.interp=0x1000 -g $(CFLAGS_W) -o $@ $<

armpe_tester.exe: armpe_tester.c $(MAKEFILE_LIST)
	arm-wince-pe-gcc -Wl,--image-base,0x400000 -s $(CFLAGS_W) -o $@ $<

mostlyclean clean distclean maintainer-clean:
	rm -f *.d *.o *.obj
	rm -f armpe_tester.exe armpe_tester.out

.PHONY: all mostlyclean clean distclean maintainer-clean

