# File: ./tests/Makefile.am
# Package: gfsm
# Description:
#   + test-level automake file
#
# Process this file with Automake to create Makefile.in.
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Options & Subdirectories
#-----------------------------------------------------------------------

## --- recursion subdirectories
#SUBDIRS =

#-----------------------------------------------------------------------
# Rules: test (check)
#-----------------------------------------------------------------------

$(srcdir)/package.m4: $(top_srcdir)/configure.in
	{                                      \
	 echo '# Signature of the current package.'; \
	 echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])'; \
	 echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])'; \
	 echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])'; \
	 echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])'; \
	 echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} >$(srcdir)/package.m4

TESTSUITE = $(srcdir)/testsuite
check-local: atconfig atlocal $(TESTSUITE)
	sh $(TESTSUITE)

AUTOTEST = autom4te --language=autotest
$(TESTSUITE): $(srcdir)/testsuite.at
	$(AUTOTEST) -I $(srcdir) $@.at -o $@.tmp
	mv $@.tmp $@

#-----------------------------------------------------------------------
# Variables: cleanup
#-----------------------------------------------------------------------

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean: built by 'make'
#CLEANFILES =

## --- distclean: built by 'configure'
DISTCLEANFILES = \
	autom4te.cache \
	config.log \
	config.status \
	config.cache \
	atconfig \
	testsuite.log

distclean-local:
	rm -rf testsuite.dir

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = *~ \
	Makefile Makefile.in \
	aclocal.m4 \
	configure

#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	atconfig \
	atlocal \
	package.m4 \
	testsuite.at \
	testsuite

## --- recursion subdirectories for 'make dist'
DIST_SUBDIRS = $(SUBDIRS)

## --- dist-hook: when another 'Makefile.am' is overkill
DISTHOOK_DIRS = data
DISTHOOK_FILES = \
	data/null.tfst \
	data/null.inf \
	data/t1.tfst \
	data/t1.inf

dist-hook:
	for d in $(DISTHOOK_DIRS); do\
	  mkdir -p $(distdir)/$$d ;\
	done
	for f in $(DISTHOOK_FILES); do\
	  cp -p $(srcdir)/$$f $(distdir)/$$f ;\
	done

#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean cvsclean-hook

cvsclean: maintainer-clean ;

