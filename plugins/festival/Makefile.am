INCLUDES = -I$(top_srcdir)/kttsd/libkttsd -I$(FESTIVAL_INCDIR) -I$(SPEECH_TOOLS_INCDIR) $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkttsd_festivalplugin.la

AM_CXXFLAGS = $(CXXFLAGS_FESTIVAL)

libkttsd_festivalplugin_la_SOURCES = \
    festivalconfwidget.ui \
    festivalconf.cpp \
    festivalproc.cpp \
    festivalplugin.cpp
libkttsd_festivalplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) -L$(SPEECH_TOOLS_LIBDIR) -L$(FESTIVAL_LIBDIR)
libkttsd_festivalplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la $(LIB_KIO) -lFestival -lestools -lestbase -leststring -lncurses

services_DATA = kttsd_festivalplugin.desktop
servicesdir = $(kde_servicesdir)

festivaldata_DATA = voices
festivaldatadir = $(kde_datadir)/kttsd/festival/

