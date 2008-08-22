
## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = cdde

cdde_SOURCES = \
	cdde.c cdde.h \
	configfile.c configfile.h \
	util.c util.h \
	string.c string.h \
	list.c list.h

cdde_LDADD = @PACKAGE_LIBS@

AUTOMAKE_OPTIONS = nostdinc
