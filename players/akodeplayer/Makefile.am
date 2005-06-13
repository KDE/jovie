INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	$(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# LIBRARY SECTION
#########################################################################
# This is the library that gets installed.  It's name is used for all
# of the other Makefile.am variables.
kde_module_LTLIBRARIES = libkttsd_akodeplugin.la

libkttsd_akodeplugin_la_SOURCES = \
	akodeplugin.cpp \
	akodeplayer.cpp
libkttsd_akodeplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) -no-undefined
libkttsd_akodeplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la -lakode

# Header files that should not be installed.
noinst_HEADERS = \
	akodeplayer.h

# This library is installed as a plugin.
services_DATA = kttsd_akodeplugin.desktop
servicesdir = $(kde_servicesdir)
