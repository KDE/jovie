LIBTOOL = $(SHELL) $(top_builddir)/libtool
INCLUDES = -I$(top_srcdir)/kttsd/libkttsd $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = libkded_kttsd.la

libkded_kttsd_la_SOURCES = kttsd.cpp kttsd.skel
libkded_kttsd_la_METASOURCES = AUTO
libkded_kttsd_la_LDFLAGS = $(all_libraries) -module -avoid-version
libkded_kttsd_la_LIBADD = $(LIB_KDECORE)  $(top_builddir)/kttsd/libkttsd/libkttsd.la

servicesdir = $(kde_servicesdir)/kded
services_DATA = kttsd.desktop

$(top_srcdir)/kttsd/libkttsd/libkttsd.la:
	@echo "Please, compile libkttsd first"
	## I hope to find a way to force the compilation of kttsd


