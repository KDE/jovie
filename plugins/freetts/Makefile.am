INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	-I$(kde_includes)/arts \
	$(all_includes)

AM_CPPFLAGS = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	-I$(kde_includes)/arts \
	$(all_includes)

METASOURCES = AUTO

kde_module_LTLIBRARIES = libkttsd_freettsplugin.la

libkttsd_freettsplugin_la_SOURCES = \
				freettsconfigwidget.ui \
				freettsconf.cpp \
				freettsproc.cpp \
				freettsplugin.cpp
libkttsd_freettsplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkttsd_freettsplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = kttsd_freettsplugin.desktop
servicesdir = $(kde_servicesdir)

# Get translators to translate library
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kttsd_freettsplugin.pot
