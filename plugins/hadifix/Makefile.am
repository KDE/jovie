INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd \
	-I$(kde_includes)/arts \
	$(all_includes)
AM_CPPFLAGS = \
	-I$(top_srcdir)/kttsd/libkttsd \
	-I$(kde_includes)/arts \
	$(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libhadifixplugin.la

libhadifixplugin_la_SOURCES = \
	hadifixconf.cpp \
	hadifixproc.cpp \
	hadifixplugin.cpp \
	hadifixconfigui.ui \
	voicefileui.ui
libhadifixplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libhadifixplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KFILE) -lartskde $(top_builddir)/kttsd/libkttsd/libkttsd.la

EXTRA_DIST = male.png female.png 

services_DATA = hadifix.desktop
servicesdir = $(kde_servicesdir)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/male.png $(DESTDIR)$(kde_icondir)/hicolor/16x16/actions/male.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/female.png $(DESTDIR)$(kde_icondir)/hicolor/16x16/actions/female.png

uninstall-local:
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/16x16/actions/male.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/16x16/actions/female.png
