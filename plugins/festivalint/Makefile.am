INCLUDES = \
        -I$(top_srcdir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)
AM_CPPFLAGS = \
        -I$(top_srcdir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkttsd_festivalintplugin.la

libkttsd_festivalintplugin_la_SOURCES = \
        festivalintconfwidget.ui \
        festivalintconf.cpp \
        festivalintproc.cpp \
        festivalintplugin.cpp
libkttsd_festivalintplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkttsd_festivalintplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la -lartskde

services_DATA = kttsd_festivalintplugin.desktop
servicesdir = $(kde_servicesdir)

# Install data files.
festivalintdatadir = $(kde_datadir)/kttsd/festivalint/
festivalintdata_DATA = voices

festivalintxsltdatadir = $(kde_datadir)/kttsd/festivalint/xslt/
festivalintxsltdata_DATA = SSMLtoSable.xsl

noinst_HEADERS =  festivalintconfwidget.h
