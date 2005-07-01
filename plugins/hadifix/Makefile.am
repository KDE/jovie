INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
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
libkttsd_hadifixplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KFILE) $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = kttsd_hadifixplugin.desktop
servicesdir = $(kde_servicesdir)

# Data files.
festivalintxsltdatadir = $(kde_datadir)/kttsd/hadifix/xslt/
festivalintxsltdata_DATA = SSMLtoTxt2pho.xsl
