INCLUDES = -I$(top_srcdir)/kttsd/libkttsd $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = kttsd

kttsd_SOURCES = main.cpp kttsd.cpp kttsd.skel speaker.cpp speechdata.cpp
kttsd_LDFLAGS = $(all_libraries) -module -avoid-version
kttsd_LDADD = $(LIB_KDECORE) $(LIB_KIO) $(top_builddir)/kttsd/libkttsd/libkttsd.la

messages:
	$(XGETTEXT) -nki18n *.cpp -o $(podir)/kttsd.pot

services_DATA = kttsd.desktop
servicesdir = $(kde_servicesdir)

