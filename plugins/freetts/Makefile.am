# This should force the compilation of kttsd first, wich is what provides libkttsd.la
COMPILE_FIRST = $(top_srcdir)/kttsd/libkttsd
LIBTOOL = $(SHELL) $(top_builddir)/libtool
INCLUDES = -I$(top_srcdir)/kttsd/libkttsd $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libfreettsplugin.la

libfreettsplugin_la_SOURCES = freettsconfigwidget.ui freettsconf.cpp freettsproc.cpp freettsplugin.cpp
libfreettsplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libfreettsplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KFILE) $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = freetts.desktop
servicesdir = $(kde_servicesdir)

$(top_srcdir)/kttsd/libkttsd/libkttsd.la:
	@echo "Please, compile libkttsd first"
	## I hope to find a way to force the compilation of kttsd
