INCLUDES = \
        -I$(top_srcdir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)
AM_CPPFLAGS = \
        -I$(top_srcdir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libfestivalintplugin.la

libfestivalintplugin_la_SOURCES = \
        festivalintconfwidget.ui \
        festivalintconf.cpp \
        festivalintproc.cpp \
        festivalintplugin.cpp
libfestivalintplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libfestivalintplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la -lartskde

services_DATA = festivalint.desktop
servicesdir = $(kde_servicesdir)

festivalintdatadir = $(kde_datadir)/kttsd/festivalint/
festivalintdata_DATA = voices

noinst_HEADERS =  festivalintconfwidget.h
