INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	-I$(kde_includes)/arts \
	$(all_includes)
AM_CPPFLAGS = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	-I$(kde_includes)/arts \
	$(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkttsd_fliteplugin.la

libkttsd_fliteplugin_la_SOURCES = \
        fliteconfwidget.ui \
        fliteconf.cpp \
        fliteproc.cpp \
        fliteplugin.cpp 
libkttsd_fliteplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkttsd_fliteplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la -lartskde

services_DATA = kttsd_fliteplugin.desktop
servicesdir = $(kde_servicesdir)

noinst_HEADERS =  fliteproc.h fliteconf.h fliteconfwidget.h

# Get translators to translate library
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kttsd_fliteplugin.pot
