INCLUDES = \
        -I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)
AM_CPPFLAGS = \
        -I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkttsd_commandplugin.la

libkttsd_commandplugin_la_SOURCES = \
        commandconfwidget.ui \
        commandconf.cpp \
        commandproc.cpp \
        commandplugin.cpp
libkttsd_commandplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkttsd_commandplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la -lartskde

services_DATA = kttsd_commandplugin.desktop
servicesdir = $(kde_servicesdir)

