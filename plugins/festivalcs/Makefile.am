INCLUDES = -I$(top_srcdir)/kttsd/libkttsd $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libfestivalcsplugin.la

libfestivalcsplugin_la_SOURCES = festivalcsconfigwidget.ui festivalcsconf.cpp festivalcsproc.cpp festivalcsplugin.cpp
libfestivalcsplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libfestivalcsplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = festivalcs.desktop
servicesdir = $(kde_servicesdir)

