INCLUDES = \
				-I$(top_srcdir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)

AM_CPPFLAGS = \
        -I$(top_srcdir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)

METASOURCES = AUTO

kde_module_LTLIBRARIES = libfreettsplugin.la

libfreettsplugin_la_SOURCES = \
				freettsconfigwidget.ui \
				freettsconf.cpp \
				freettsproc.cpp \
				freettsplugin.cpp
libfreettsplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libfreettsplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = freetts.desktop
servicesdir = $(kde_servicesdir)
