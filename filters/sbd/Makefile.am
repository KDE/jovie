INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	$(all_includes)

METASOURCES = AUTO

kde_module_LTLIBRARIES = libkttsd_sbdplugin.la

libkttsd_sbdplugin_la_SOURCES = \
	sbdproc.cpp \
	sbdconf.cpp \
	sbdplugin.cpp

libkttsd_sbdplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkttsd_sbdplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = kttsd_sbdplugin.desktop
servicesdir = $(kde_servicesdir)

# Install data files.
sbddatadir = $(kde_datadir)/kttsd/sbd/
sbddata_DATA = 

noinst_HEADERS = sbdproc.h

# Get translators to translate library.
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kttsd_sbdplugin.pot
