INCLUDES = -I$(top_srcdir)/kttsd/libkttsd $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libcommandplugin.la

libcommandplugin_la_SOURCES = commandconf.cpp commandproc.cpp commandplugin.cpp texttospeechconfigurationui.ui texttospeechconfigurationwidget.cpp texttospeechsystem.cpp speech.cpp
libcommandplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libcommandplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = command.desktop
servicesdir = $(kde_servicesdir)

