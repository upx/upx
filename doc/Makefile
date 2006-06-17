MAKEFLAGS += -rR
.SUFFIXES:
SHELL = /bin/sh

top_srcdir = ..

PACKAGE       = upx
VERSION_DATE := $(shell sed -n 's/^.*UPX_VERSION_DATE_ISO .*"\(.*\)".*/\1/p' $(top_srcdir)/src/version.h)
VERSION      := $(shell sed -n 's/^.*UPX_VERSION_STRING .*"\(.*\)".*/\1/p' $(top_srcdir)/src/version.h)

TRIMSPACE = sed -e 's/[ 	]*$$//'
DETAB2    = sed -e 's/	/  /g'

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
	pod2man --center=" " --release="$(PACKAGE) $(VERSION)" --date="$(VERSION_DATE)" $< | $(TRIMSPACE) > $@

%.doc : %.pod
	pod2text < $< | $(TRIMSPACE) > $@

%.html : %.pod
	pod2html --noindex $< | $(TRIMSPACE) | $(DETAB2) > $@
	@rm -f pod2htm*

%.man : %.1
	nroff -man $< | $(TRIMSPACE) > $@

%.ps : %.1
	groff -man $< | $(TRIMSPACE) > $@

%.tex : %.pod
	pod2latex $<


###
### dependencies
###

$(BUILT_SOURCES): $(top_srcdir)/src/version.h $(MAKEFILE_LIST)

