#  Makefile for sstrip

SHELL = /bin/sh

##all: amd64-linux-sstrip
##all: i386-linux-sstrip
all: sstrip

sstrip: sstrip.c
	gcc -O2 -g -Wall -W -o $@ $<
amd64-linux-sstrip: sstrip.c
	gcc -m64 -O2 -g -Wall -W -o $@ $<
i386-linux-sstrip: sstrip.c
	gcc -m32 -O2 -g -Wall -W -o $@ $<

mostlyclean clean distclean maintainer-clean:
	rm -f *.d *.o *.obj
	rm -f sstrip amd64-linux-sstrip i386-linux-sstrip

.PHONY: all mostlyclean clean distclean maintainer-clean

