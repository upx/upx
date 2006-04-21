#  Makefile for armpe_tester

SHELL = /bin/sh

all: armpe_tester

armpe_tester: armpe_tester.c
	arm-9tdmi-linux-gnu-gcc -Wl,--section-start,.interp=0x1000 -g -Wall -W -o $@ $<

mostlyclean clean distclean maintainer-clean:
	rm -f *.d *.o *.obj
	rm -f  armpe_tester

.PHONY: all mostlyclean clean distclean maintainer-clean

