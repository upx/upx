#
# UPX doc Makefile - needs GNU make 3.80 or better
#

MAKEFLAGS += -rR
.SUFFIXES:
SHELL = /bin/sh

# internal make variables
comma := ,
empty :=
space := $(empty) $(empty)
tab   := $(empty)	$(empty)

# update $PATH for our special stub build tools
ifneq ($(wildcard $(HOME)/local/bin/bin-upx/.),)
export PATH := $(HOME)/local/bin/bin-upx:$(PATH)
endif
ifneq ($(wildcard $(HOME)/bin/bin-upx/.),)
export PATH := $(HOME)/bin/bin-upx:$(PATH)
endif


# /***********************************************************************
# //
# ************************************************************************/

top_srcdir ?= ..

PACKAGE       = upx
VERSION_DATE := $(shell sed -n 's/^.*UPX_VERSION_DATE_ISO .*"\(.*\)".*/\1/p' $(top_srcdir)/src/version.h)
VERSION      := $(shell sed -n 's/^.*UPX_VERSION_STRING .*"\(.*\)".*/\1/p' $(top_srcdir)/src/version.h)

RTRIM  := sed -e 's/[ $(tab)]*$$//'
DETAB2 := sed -e 's/$(tab)/  /g'

BUILT_SOURCES = upx.1 upx.doc upx.html upx.man upx.ps upx.tex
BUILT_SOURCES = upx.1 upx.doc upx.html


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

.SUFFIXES: .1 .doc .html .man .pod .ps .tex

%.1 : %.pod
	pod2man --center=" " --release="$(PACKAGE) $(VERSION)" --date="$(VERSION_DATE)" $< | $(RTRIM) > $@

%.doc : %.pod
	pod2text < $< | $(RTRIM) > $@

%.html : %.pod
	pod2html --noindex $< | $(RTRIM) | $(DETAB2) > $@
	@rm -f pod2htm*

%.man : %.1
	nroff -man $< | $(RTRIM) > $@

%.ps : %.1
	groff -man $< | $(RTRIM) > $@

%.tex : %.pod
	pod2latex $<


###
### dependencies
###

$(BUILT_SOURCES): $(top_srcdir)/src/version.h $(MAKEFILE_LIST)
.DELETE_ON_ERROR: $(BUILT_SOURCES)

