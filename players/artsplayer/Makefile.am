# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = \
	-I$(kde_includes)/arts \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	$(all_includes)
	
AM_CPPFLAGS = \
	-I$(kde_includes)/arts \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	$(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# LIBRARY SECTION
#########################################################################
# This is the library that gets installed.  It's name is used for all
# of the other Makefile.am variables.
kde_module_LTLIBRARIES = libkttsd_artsplugin.la

libkttsd_artsplugin_la_SOURCES = \
    artsplugin.cpp \
	artsplayer.cpp
libkttsd_artsplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) -no-undefined
libkttsd_artsplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la -lartskde

# Header files that should not be installed.
noinst_HEADERS = \
	artsplayer.h

# This library is installed as a plugin.
services_DATA = kttsd_artsplugin.desktop
servicesdir = $(kde_servicesdir)

