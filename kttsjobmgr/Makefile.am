# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = -I$(top_srcdir)/kttsd/libktts -I$(top_builddir)/kttsd/libktts $(all_includes)
AM_CPPFLAGS = -I$(top_srcdir)/kttsd/libktts $(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# KPART SECTION
#########################################################################
# This is the kpart that gets installed.  It's name is used for all
# of the other Makefile.am variables.
kde_module_LTLIBRARIES =  libkttsjobmgr.la

# The source, library search path, and link libraries.
libkttsjobmgr_la_SOURCES = kttsjobmgr.cpp
libkttsjobmgr_la_LDFLAGS = -avoid-version -module -no-undefined $(all_libraries) $(KDE_PLUGIN)
libkttsjobmgr_la_LIBADD = $(top_builddir)/kttsd/libktts/libktts.la $(LIB_KFILE) $(LIBVM) $(LIB_KPARTS)

# Header files to be installed.
kpartsincludedir = $(includedir)/kparts
kpartsinclude_HEADERS = kttsjobmgr.h

# Install desktop file to standard services directory.
kde_services_DATA =  kttsjobmgr.desktop

# KPart data.
rcdir = $(kde_datadir)/kttsjobmgr
rc_DATA = kttsjobmgrui.rc

# Translation message files.
messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kttsjobmgr.pot
