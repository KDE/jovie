INCLUDES = \
        -I$(top_srcdir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)
AM_CPPFLAGS = \
        -I$(top_srcdir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libfliteplugin.la

libfliteplugin_la_SOURCES = \
        fliteconfwidget.ui \
        fliteconf.cpp \
        fliteproc.cpp \
        fliteplugin.cpp 
libfliteplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libfliteplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la -lartskde

services_DATA = flite.desktop
servicesdir = $(kde_servicesdir)

noinst_HEADERS =  fliteproc.h fliteconf.h fliteconfwidget.h
