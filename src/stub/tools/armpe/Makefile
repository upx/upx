#  Makefile for armpe_tester

MAKEFLAGS += -rR
SHELL = /bin/sh

# update path for our special stub build tools
ifneq ($(wildcard $(HOME)/local/bin/bin-upx),)
export PATH := $(HOME)/local/bin/bin-upx:$(PATH)
endif

ifneq ($(wildcard $(HOME)/bin/bin-upx),)
export PATH := $(HOME)/bin/bin-upx:$(PATH)
endif


all: armpe_tester wtest.exe

armpe_tester: armpe_tester.c
	arm-9tdmi-linux-gnu-gcc -Wl,--section-start,.interp=0x1000 -g -Wall -W -o $@ $<

wtest.exe:  armpe_tester.c
	arm-wince-pe-gcc -Wl,--image-base,0x400000 -s -Wall -W -o $@ $<

mostlyclean clean distclean maintainer-clean:
	rm -f *.d *.o *.obj
	rm -f  armpe_tester wtest.exe

.PHONY: all mostlyclean clean distclean maintainer-clean

