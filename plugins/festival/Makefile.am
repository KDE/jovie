## This should force the compilation of proklam first, wich is what provides libproklam.la
COMPILE_FIRST = $(top_srcdir)/proklam/libproklam
LIBTOOL = $(SHELL) $(top_builddir)/libtool
INCLUDES = -I$(top_srcdir)/proklam/libproklam -I$(SPEECH_TOOLS_INCDIR) -I$(FESTIVAL_INCDIR) $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libfestivalplugin.la

AM_CXXFLAGS = $(CXXFLAGS_FESTIVAL)

libfestivalplugin_la_SOURCES = festivalconfwidget.ui festivalconf.cpp festivalproc.cpp festivalplugin.cpp
libfestivalplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) -L$(SPEECH_TOOLS_LIBDIR) -L$(FESTIVAL_LIBDIR)
libfestivalplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KFILE) -lFestival -lestools -lestbase -leststring -lncurses $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = festival.desktop
servicesdir = $(kde_servicesdir)

festivaldata_DATA = voices
festivaldatadir = $(kde_datadir)/kttsd/festival/

$(top_srcdir)/kttsd/libkttsd/libkttsd.la:
	@echo "Please, compile libproklam first"
	## I hope to find a way to force the compilation of proklam
