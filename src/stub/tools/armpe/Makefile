#  Makefile for armpe_tester

CFLAGS_W = -Wall -Wextra -Wcast-align -Wcast-qual -Wwrite-strings

all: armpe_tester.out
#all: armpe_tester.exe

armpe_tester.out: armpe_tester.c $(MAKEFILE_LIST)
	arm-linux-gnueabi-gcc -static $(CFLAGS_W) -o $@ $<

armpe_tester.exe: armpe_tester.c $(MAKEFILE_LIST)
	arm-wince-pe-gcc -Wl,--image-base,0x400000 -s $(CFLAGS_W) -o $@ $<

mostlyclean clean distclean maintainer-clean:
	rm -f *.d *.o *.obj
	rm -f armpe_tester.exe armpe_tester.out

.PHONY: all mostlyclean clean distclean maintainer-clean
