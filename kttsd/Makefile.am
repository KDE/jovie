AM_CPPFLAGS = -I$(top_srcdir)/kttsd/libkttsd -I$(top_srcdir)/kttsd/libktts $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = kttsd

kttsd_SOURCES = main.cpp kttsd.cpp speaker.cpp speechdata.cpp kttsd.skel
kttsd_LDFLAGS = -avoid-version -module $(all_libraries)
kttsd_LDADD = $(top_builddir)/kttsd/libktts/libktts.la $(top_builddir)/kttsd/libkttsd/libkttsd.la $(LIB_KDECORE) $(LIB_KIO) $(LIB_KUTILS)

messages: rc.cpp
	$(XGETTEXT) -nki18n *.cpp -o $(podir)/kttsd.pot

services_DATA = kttsd.desktop
servicesdir = $(kde_servicesdir)

INCLUDES = -I$(top_srcdir)/kttsd/libktts -I$(top_srcdir)/kttsd/libkttsd
