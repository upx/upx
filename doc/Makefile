SHELL = /bin/sh

top_srcdir = ..

PACKAGE      = upx
VERSION_DATE = 10 May 2000
VERSION     := $(shell sed -n 's/^.*UPX_VERSION_STRING.*"\(.*\)".*/\1/p' $(top_srcdir)/src/version.h)

TRIMSPACE = cat
TRIMSPACE = sed -e 's/ *$$//'

BUILT_SOURCES = upx.1 upx.doc upx.html upx.man upx.ps upx.tex


###
###
###

all: $(BUILT_SOURCES)

clean:
	-rm -f $(BUILT_SOURCES) pod2html*cache

distclean: clean

.SUFFIXES: .1 .doc .html .man .pod .ps .tex .texi

.PHONY: all clean distclean


###
### rules
###

.pod.1:
	pod2man --center=" " --release="$(PACKAGE) $(VERSION)" --date="$(VERSION_DATE)" $< | $(TRIMSPACE) > $@

.pod.html:
###	pod2html $< | $(TRIMSPACE) > $@
	pod2html --noindex $< | $(TRIMSPACE) > $@
	@-rm -f pod2html*cache

.pod.tex:
	pod2latex $<

.pod.doc:
	pod2text < $< | $(TRIMSPACE) > $@

.1.man:
	nroff -man $< | $(TRIMSPACE) > $@

.1.ps:
	groff -man $< | $(TRIMSPACE) > $@


###
### dependencies
###

$(BUILT_SOURCES): $(top_srcdir)/src/version.h Makefile

.NOEXPORT:

