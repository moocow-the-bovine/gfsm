## File: doc/programs/Makefile.am
## Package: mootm
## Description:
##   + program-doc-level automake file for moocow's PoS tagger programs
##
## Process this file with Automake to create Makefile.in.
##-----------------------------------------------------------------------

##-----------------------------------------------------------------------
## subdirectories
##-----------------------------------------------------------------------
#SUBDIRS =

##-----------------------------------------------------------------------
## primary targets
##-----------------------------------------------------------------------

## --- manpages (section 1)
man1_MANS = @DOC_PROG_MAN1_TARGETS@
man5_MANS = @DOC_PROG_MAN5_TARGETS@

pkgdocprog_DATA = \
	@DOC_PROG_TXT_TARGETS@ \
	@DOC_PROG_HTML_TARGETS@ \
	@DOC_PROG_DVI_TARGETS@ \
	@DOC_PROG_PS_TARGETS@ \
	@DOC_PROG_PDF_TARGETS@

##-----------------------------------------------------------------------
## sources
##-----------------------------------------------------------------------

SUFFIXES = .skel .gog .pod .txt .1 .5 .html .tex .dvi .ps .pdf

##-----------------------------------------------------------------------
## local hooks
##-----------------------------------------------------------------------
all-local: $(man_MANS)

txt: $(gfsm_program_gogs:.gog=.txt)

man: $(man1_MANS) $(MAN5_MANS)

html: $(gfsm_program_gogs:.gog=.html)

latex: $(gfsm_program_gogs:.gog=.tex)

dvi: $(gfsm_program_gogs:.gog=.dvi)

ps: $(gfsm_program_gogs:.gog=.ps)

pdf: $(gfsm_program_gogs:.gog=.pdf)


##-----------------------------------------------------------------------
## pod compilation rules: Getopt::Gen
##-----------------------------------------------------------------------

.SECONDARY:

OPTGENFLAGS = -l

.gog.pod:
	optgen.perl $(OPTGENFLAGS) --nocfile --nohfile --no-handle-rcfile -F $* $<

##-----------------------------------------------------------------------
## pod compilation rules: Getopt::Gen (from skel)
##-----------------------------------------------------------------------
OPTGEN_SKELFLAGS = \
	--nocfile --nohfile --nopod \
	--no-handle-help \
	--no-handle-version \
	--no-handle-rcfile \
	--no-handle-error

.skel.pod:
	echo | optgen.perl $(OPTGEN_SKELFLAGS) --template=$< - > $@ 2>/dev/null

##-----------------------------------------------------------------------
## pod conversion rules
##-----------------------------------------------------------------------

POD2MANFLAGS = \
	--center="GFSM Utilities" \
	--release="GFSM v$(VERSION)"
POD2HTMLFLAGS = --podpath="$(srcdir):." --htmlroot="."

.pod.1:
	$(POD2MAN) $(POD2MANFLAGS) --section=1 $< $@

.pod.5:
	$(POD2MAN) $(POD2MANFLAGS) --section=5 $< $@

.pod.txt:
	$(POD2TEXT) $< $@

.pod.html:
	$(POD2HTML) $(POD2HTMLFLAGS) --infile=$< --outfile=$@

.pod.tex:
	$(POD2LATEX) --full --out $@ $<

##-----------------------------------------------------------------------
## latex compilation
##-----------------------------------------------------------------------
.tex.dvi:
	latex $*
	latex $*

.dvi.ps:
	dvips $*

.tex.pdf:
	pdflatex $*
	pdflatex $*

##-----------------------------------------------------------------------
## Variables: cleanup
##-----------------------------------------------------------------------

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean:  built by 'make'
CLEANFILES = \
	$(filter-out acknowledge.pod,$(wildcard *.pod)) \
	*.man *.1 *.5 *.html *.txt *.tex *.dvi *.ps *.pdf \
	*.log *.aux *.toc *.idx *.tmp *~~

## --- distclean: built by 'configure'
DISTCLEANFILES = \
	*.pod *.gog

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = \
	*~ .*~ Makefile Makefile.in \
	*.pod *.man *.1 *.html *.txt *.tex *.dvi *.ps *.pdf

##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	gfsmutils.skel \
	@DOC_PROG_TXT_TARGETS@ \
	@DOC_PROG_MAN1_TARGETS@ \
	@DOC_PROG_MAN5_TARGETS@ \
	@DOC_PROG_HTML_TARGETS@

## --- recursion subdirectories for 'make dist'
#DIST_SUBDIRS = $(SUBDIRS)

## --- dist-hook: when another 'Makefile.am' is overkill
#DISTHOOK_DIRS = foo
#DISTHOOK_FILES = foo/bar.txt foo/baz.txt
#dist-hook:
#	for d in $(DISTHOOK_DIRS); do\
#	  mkdir -p $(distdir)/$$d ;\
#	done
#	for f in $(DISTHOOK_FILES); do\
#	  cp -p $(srcdir)/$$f $(distdir)/$$f ;\
#	done

##-----------------------------------------------------------------------
## Rules: cleanup
##-----------------------------------------------------------------------
.PHONY: cvsclean

cvsclean: maintainer-clean ;
