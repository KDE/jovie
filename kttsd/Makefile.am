# This should force the compilation of kttsd first, wich is what provides libkttsd.la
COMPILE_FIRST = $(top_srcdir)/kttsd/libkttsd
LIBTOOL = $(SHELL) $(top_builddir)/libtool
INCLUDES = -I$(top_srcdir)/kttsd/libkttsd $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kded_kttsd.la

kded_kttsd_la_SOURCES = kttsd.cpp kttsd.skel speaker.cpp speechdata.cpp
kded_kttsd_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_kttsd_la_LIBADD = $(LIB_KDECORE)  $(top_builddir)/kttsd/libkttsd/libkttsd.la

servicesdir = $(kde_servicesdir)/kded
services_DATA = kttsd.desktop

$(top_srcdir)/kttsd/libkttsd/libkttsd.la:
	@echo "Please, compile libkttsd first"
	## I hope to find a way to force the compilation of kttsd


