INCLUDES = \
        -I$(top_srcdir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)
AM_CPPFLAGS = \
        -I$(top_srcdir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libeposkttsdplugin.la

libeposkttsdplugin_la_SOURCES = \
        eposconfwidget.ui \
        eposconf.cpp \
        eposproc.cpp \
        eposplugin.cpp 
libeposkttsdplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libeposkttsdplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la -lartskde

services_DATA = epos-kttsdplugin.desktop
servicesdir = $(kde_servicesdir)

noinst_HEADERS =  eposproc.h eposconf.h eposconfwidget.h
