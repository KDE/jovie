INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	-I$(kde_includes)/arts \
	$(all_includes)

METASOURCES = AUTO

kde_module_LTLIBRARIES = libkttsd_hadifixplugin.la

libkttsd_hadifixplugin_la_SOURCES = \
	hadifixconf.cpp \
	hadifixproc.cpp \
	hadifixplugin.cpp \
	hadifixconfigui.ui \
	voicefileui.ui
libkttsd_hadifixplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkttsd_hadifixplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KFILE) -lartskde $(top_builddir)/kttsd/libkttsd/libkttsd.la

EXTRA_DIST = male.png female.png 

services_DATA = kttsd_hadifixplugin.desktop
servicesdir = $(kde_servicesdir)

# Data files.
festivalintxsltdatadir = $(kde_datadir)/kttsd/hadifix/xslt/
festivalintxsltdata_DATA = SSMLtoTxt2pho.xsl

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/male.png $(DESTDIR)$(kde_icondir)/hicolor/16x16/actions/male.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/female.png $(DESTDIR)$(kde_icondir)/hicolor/16x16/actions/female.png

uninstall-local:
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/16x16/actions/male.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/16x16/actions/female.png
