## This should force the compilation of proklam first, wich is what provides libproklam.la
COMPILE_FIRST = $(top_srcdir)/proklam/libproklam
LIBTOOL = $(SHELL) $(top_builddir)/libtool
INCLUDES = -I$(top_srcdir)/proklam/libproklam $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libcommandplugin.la

libcommandplugin_la_SOURCES = commandconf.cpp commandproc.cpp commandplugin.cpp texttospeechconfigurationui.ui texttospeechconfigurationwidget.cpp texttospeechsystem.cpp speech.cpp
libcommandplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libcommandplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KFILE) $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = command.desktop
servicesdir = $(kde_servicesdir)

$(top_srcdir)/kttsd/libkttsd/libkttsd.la:
	@echo "Please, compile libproklam first"
	## I hope to find a way to force the compilation of proklam
