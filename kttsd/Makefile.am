INCLUDES = -I$(top_srcdir)/kttsd/libkttsd $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = kttsd

kttsd_SOURCES = main.cpp kttsdui.ui kttsd.cpp kspeech.skel speaker.cpp speechdata.cpp
kttsd_LDFLAGS = -avoid-version -module $(all_libraries)
kttsd_LDADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la $(LIB_KDECORE) $(LIB_KIO) $(LIB_KUTILS)

messages:
	$(XGETTEXT) -nki18n *.cpp -o $(podir)/kttsd.pot

services_DATA = kttsd.desktop
servicesdir = $(kde_servicesdir)

noinst_HEADERS =  kspeech.h
include_HEADERS =  kspeech.h
