# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd \
	-I$(kde_includes)/arts \
	$(CFLAGS_GST) \
	$(all_includes)

SUBDIRS = stringreplacer xmltransformer sbd

#########################################################################
# APPLICATION SECTION
#########################################################################
# This is the program that gets installed.  It's name is used for all
# of the other Makefile.am variables.
check_PROGRAMS = testfilter

testfilter_SOURCES = main.cpp

testfilter_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testfilter_LDADD = \
	$(top_builddir)/kttsd/libkttsd/libkttsd.la \
	$(LIB_KDECORE)

kde_servicetypes_DATA = kttsd_filterplugin.desktop

