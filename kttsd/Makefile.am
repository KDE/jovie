# This should force the compilation of kttsd first, wich is what provides libkttsd.la
COMPILE_FIRST = $(top_srcdir)/kttsd/libkttsd
LIBTOOL = $(SHELL) $(top_builddir)/libtool
INCLUDES = -I$(top_srcdir)/kttsd/libkttsd $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = kttsd

kttsd_SOURCES = main.cpp kttsd.cpp kttsd.skel speaker.cpp speechdata.cpp
kttsd_LDFLAGS = $(all_libraries) -module -avoid-version
kttsd_LDADD = $(LIB_KDECORE)  $(top_builddir)/kttsd/libkttsd/libkttsd.la

messages:
	$(XGETTEXT) -nki18n $(kttsd_SOURCES) -o $(podir)/kttsd.pot

services_DATA = kttsd.desktop
servicesdir = $(kde_servicesdir)

$(top_srcdir)/kttsd/libkttsd/libkttsd.la:
	@echo "Please, compile libkttsd first"
	## I hope to find a way to force the compilation of kttsd
