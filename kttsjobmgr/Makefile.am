INCLUDES = -I$(top_srcdir)/kttsd/libktts
AM_CPPFLAGS = -I$(top_srcdir)/kttsd/libktts $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES =  libkttsjobmgr.la
libkttsjobmgr_la_SOURCES = kttsjobmgr.cpp
libkttsjobmgr_la_LDFLAGS = -avoid-version -module -no-undefined $(all_libraries) $(KDE_PLUGIN)
libkttsjobmgr_la_LIBADD = $(top_builddir)/kttsd/libktts/libktts.la $(LIB_KFILE) $(LIBVM) $(LIB_KPARTS)

noinst_HEADERS = kttsjobmgr.h

kde_services_DATA =  kttsjobmgr.desktop

rcdir = $(kde_datadir)/kttsjobmgr
rc_DATA = kttsjobmgrui.rc

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kttsjobmgr.pot
