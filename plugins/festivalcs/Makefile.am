INCLUDES = -I$(top_srcdir)/kttsd/libkttsd $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkttsd_festivalcsplugin.la

libkttsd_festivalcsplugin_la_SOURCES = \
    festivalcsconfigwidget.ui \
    festivalcsconf.cpp \
    festivalcsproc.cpp \
    festivalcsplugin.cpp
libkttsd_festivalcsplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkttsd_festivalcsplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = kttsd_festivalcsplugin.desktop
servicesdir = $(kde_servicesdir)

