LIBTOOL = $(SHELL) $(top_builddir)/libtool
INCLUDES = $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = libkttsd.la

libkttsd_la_SOURCES = pluginproc.cpp pluginconf.cpp
libkttsd_la_LDFLAGS = -version-info 1:0:0 -module  $(all_libraries)
libkttsd_la_LIBADD =

servicetypes_DATA = kttsd.desktop
servicetypesdir = $(kde_servicetypesdir)
