INCLUDES = -I$(top_srcdir)/kttsd/libktts
AM_CPPFLAGS = -I$(top_srcdir)/kttsd/libktts $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES =  kcm_kttsmgr.la
kcm_kttsmgr_la_LDFLAGS = -avoid-version -module -no-undefined 
kcm_kttsmgr_la_SOURCES = kcmkttsmgr.cpp kcmkttsmgrwidget.ui

noinst_HEADERS = kcmkttsmgr.h kcmkttsmgrwidget.h
kcm_kttsmgr_la_LIBADD = $(top_builddir)/kttsd/libktts/libktts.la $(top_builddir)/kttsd/libkttsd/libkttsd.la $(LIB_QT) $(LIB_KIO) $(LIB_KDECORE)

kde_services_DATA =  kcmkttsmgr.desktop

apps_DATA = kcmkttsmgr.desktop
appsdir = $(kde_appsdir)/Settings/Accessibility

xdg_apps_DATA = kcmkttsmgr.desktop

messages: rc.cpp
	$(XGETTEXT) -nki18n *.cpp -o $(podir)/kcmkttsmgr.pot

