# This should force the compilation of kttsd first, wich is what provides libkttsd.la
COMPILE_FIRST = $(top_srcdir)/kttsd/libkttsd
LIBTOOL = $(SHELL) $(top_builddir)/libtool
INCLUDES = -I$(top_srcdir)/kttsd/libkttsd $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libfestivalcsplugin.la

libfestivalcsplugin_la_SOURCES = festivalcsconfigwidget.ui festivalcsconf.cpp festivalcsproc.cpp festivalcsplugin.cpp
libfestivalcsplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libfestivalcsplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KFILE) $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = festivalcs.desktop
servicesdir = $(kde_servicesdir)

## festivalcsdata_DATA = voices
## festivalcsdatadir = $(kde_datadir)/kttsd/festivalcs/

$(top_srcdir)/kttsd/libkttsd/libkttsd.la:
	@echo "Please, compile libkttsd first"
	## I hope to find a way to force the compilation of kttsd
