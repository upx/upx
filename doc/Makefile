#
# UPX doc Makefile - needs GNU make, pod2html, pod2man and pod2text
#

BUILT_SOURCES = upx.1 upx-doc.html upx-doc.txt

top_srcdir ?= ..

PACKAGE       = upx
VERSION_DATE := $(shell sed -n 's/^.*UPX_VERSION_DATE_ISO .*"\(.*\)".*/\1/p' $(top_srcdir)/src/version.h)
VERSION      := $(shell sed -n 's/^.*UPX_VERSION_STRING .*"\(.*\)".*/\1/p' $(top_srcdir)/src/version.h)

# internal make variables
comma := ,
empty :=
space := $(empty) $(empty)
tab   := $(empty)	$(empty)

RTRIM  := sed -e 's/[ $(tab)]*$$//'
DETAB2 := sed -e 's/$(tab)/  /g'

#***********************************************************************
# targets
#***********************************************************************

.DEFAULT_GOAL = all

all: $(BUILT_SOURCES) PHONY

mostlyclean clean distclean maintainer-clean: PHONY
	rm -f $(BUILT_SOURCES) ./pod2htm*

#***********************************************************************
# rules
#***********************************************************************

.SUFFIXES:
.SUFFIXES: .1 .html .man .pod .ps .tex .txt

%.1 : %.pod
	pod2man --center=" " --release="$(PACKAGE) $(VERSION)" --date="$(VERSION_DATE)" $< | $(RTRIM) > $@
	test -s $@

%-doc.html : %.pod
	pod2html --noindex $< | $(RTRIM) | $(DETAB2) > $@
	@rm -f ./pod2htm*
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

#***********************************************************************
# dependencies
#***********************************************************************

$(BUILT_SOURCES): $(top_srcdir)/src/version.h $(MAKEFILE_LIST)
.DELETE_ON_ERROR: $(BUILT_SOURCES)
.PHONY: PHONY
