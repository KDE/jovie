INCLUDES = -I$(top_srcdir)/kttsd/libktts
AM_CPPFLAGS = -I$(top_srcdir)/kttsd/libktts $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = kttsmgr
kttsmgr_LDFLAGS = $(all_libraries)
kttsmgr_SOURCES =  kttsmgr.cpp
kttsmgr_LDADD = $(top_builddir)/kttsd/libktts/libktts.la $(LIB_KDECORE) $(LIB_KUTILS) $(LIB_KDEUI)

noinst_HEADERS = kttsmgr.h

xdg_apps_DATA = kttsmgr.desktop

applnkApplicationdir = $(kde_appsdir)/Application
applnkApplication_DATA = kttsmgr.desktop

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kttsmgr.pot

