INCLUDES = \
        -I$(top_srcdir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)
AM_CPPFLAGS = \
        -I$(top_srcdir)/kttsd/libkttsd \
        -I$(kde_includes)/arts \
        $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libcommandplugin.la

libcommandplugin_la_SOURCES = \
        commandconfwidget.ui \
        commandconf.cpp \
        commandproc.cpp \
        commandplugin.cpp
libcommandplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libcommandplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la -lartskde

services_DATA = command.desktop
servicesdir = $(kde_servicesdir)

