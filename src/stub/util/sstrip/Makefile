#  Makefile for sstrip

SHELL = /bin/sh

all: sstrip

sstrip: sstrip.c
	gcc -O2 -g -Wall -W -o $@ $<

mostlyclean:
	-rm -f *.d *.err *.i *.log *.map *~ gdb-trans*

clean distclean maintainer-clean: mostlyclean
	-rm -f *.a *.lib *.o *.obj tags TAGS ID
	-rm -rf .deps
	rm -f sstrip

.PHONY: all mostlyclean clean distclean maintainer-clean

.NOEXPORT:

# vi:nowrap

