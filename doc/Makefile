#
# UPX doc Makefile - needs GNU make
#

MAKEFLAGS += -rR
.SUFFIXES:
SHELL = /bin/sh

# internal make variables
comma := ,
empty :=
space := $(empty) $(empty)
tab   := $(empty)	$(empty)


# /***********************************************************************
# //
# ************************************************************************/

top_srcdir ?= ..

PACKAGE       = upx
VERSION_DATE := $(shell sed -n 's/^.*UPX_VERSION_DATE_ISO .*"\(.*\)".*/\1/p' $(top_srcdir)/src/version.h)
VERSION      := $(shell sed -n 's/^.*UPX_VERSION_STRING .*"\(.*\)".*/\1/p' $(top_srcdir)/src/version.h)

RTRIM  := sed -e 's/[ $(tab)]*$$//'
DETAB2 := sed -e 's/$(tab)/  /g'

BUILT_SOURCES = upx.1 upx-doc.html upx-doc.txt


###
### targets
###

all: $(BUILT_SOURCES)

mostlyclean clean distclean maintainer-clean:
	rm -f $(BUILT_SOURCES) pod2htm*

.PHONY: all mostlyclean clean distclean maintainer-clean


###
### rules
###

.SUFFIXES: .1 .html .man .pod .ps .tex .txt

%.1 : %.pod
	pod2man --center=" " --release="$(PACKAGE) $(VERSION)" --date="$(VERSION_DATE)" $< | $(RTRIM) > $@
	test -s $@

%-doc.html : %.pod
	pod2html --noindex $< | $(RTRIM) | $(DETAB2) > $@
	@rm -f pod2htm*
	test -s $@

%.man : %.1
	nroff -man $< | $(RTRIM) > $@
	test -s $@

%.ps : %.1
	groff -man $< | $(RTRIM) > $@
	test -s $@

%.tex : %.pod
	pod2latex $<
	test -s $@

%-doc.txt : %.pod
	pod2text < $< | $(RTRIM) > $@
	test -s $@


###
### dependencies
###

$(BUILT_SOURCES): $(top_srcdir)/src/version.h $(MAKEFILE_LIST)
.DELETE_ON_ERROR: $(BUILT_SOURCES)

