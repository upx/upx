#  Makefile for armpe_tester

SHELL = /bin/sh

all: armpe_tester wtest.exe

armpe_tester: armpe_tester.c
	arm-9tdmi-linux-gnu-gcc -Wl,--section-start,.interp=0x1000 -g -Wall -W -o $@ $<

wtest.exe:  armpe_tester.c
	arm-wince-pe-gcc -Wl,--image-base,0x400000 -s -Wall -W -o $@ $<

mostlyclean clean distclean maintainer-clean:
	rm -f *.d *.o *.obj
	rm -f  armpe_tester

.PHONY: all mostlyclean clean distclean maintainer-clean

