# Include paths.
INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	$(all_includes)
KDE_CXXFLAGS = -D_LARGEFILE64_SOURCE -D_GNU_SOURCE

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# LIBRARY SECTION
#########################################################################
# This is the library that gets installed.  It's name is used for all
# of the other Makefile.am variables.
kde_module_LTLIBRARIES = libkttsd_alsaplugin.la

libkttsd_alsaplugin_la_SOURCES = \
    alsaplugin.cpp \
	alsaplayer.cpp
libkttsd_alsaplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(LIBASOUND) -no-undefined
libkttsd_alsaplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la

# Header files that should not be installed.
noinst_HEADERS = \
	alsaplayer.h \
	formats.h

# This library is installed as a plugin.
services_DATA = kttsd_alsaplugin.desktop
servicesdir = $(kde_servicesdir)
