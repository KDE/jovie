## This should force the compilation of proklam first, wich is what provides libproklam.la
COMPILE_FIRST = $(top_srcdir)/proklam/libproklam
LIBTOOL = $(SHELL) $(top_builddir)/libtool
INCLUDES = -I$(top_srcdir)/proklam/libproklam $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libfestivalcsplugin.la

libfestivalcsplugin_la_SOURCES = festivalcsconfigwidget.ui festivalcsconf.cpp festivalcsproc.cpp festivalcsplugin.cpp
libfestivalcsplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libfestivalcsplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KFILE) $(top_builddir)/proklam/libproklam/libproklam.la

services_DATA = festivalcs.desktop
servicesdir = $(kde_servicesdir)

## festivalcsdata_DATA = voices
## festivalcsdatadir = $(kde_datadir)/proklam/festivalcs/

$(top_srcdir)/proklam/libproklam/libproklam.la:
	@echo "Please, compile libproklam first"
	## I hope to find a way to force the compilation of proklam
