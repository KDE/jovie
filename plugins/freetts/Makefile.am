## This should force the compilation of proklam first, wich is what provides libproklam.la
COMPILE_FIRST = $(top_srcdir)/proklam/libproklam
LIBTOOL = $(SHELL) $(top_builddir)/libtool
INCLUDES = -I$(top_srcdir)/proklam/libproklam $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libfreettsplugin.la

libfreettsplugin_la_SOURCES = freettsconfigwidget.ui freettsconf.cpp freettsproc.cpp freettsplugin.cpp
libfreettsplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libfreettsplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KFILE) $(top_builddir)/proklam/libproklam/libproklam.la

services_DATA = freetts.desktop
servicesdir = $(kde_servicesdir)

$(top_srcdir)/proklam/libproklam/libproklam.la:
	@echo "Please, compile libproklam first"
	## I hope to find a way to force the compilation of proklam
