INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	$(all_includes)

METASOURCES = AUTO

kde_module_LTLIBRARIES = libkttsd_xmltransformerplugin.la

libkttsd_xmltransformerplugin_la_SOURCES = \
	xmltransformerconfwidget.ui \
	xmltransformerconf.cpp \
	xmltransformerproc.cpp \
	xmltransformerplugin.cpp
libkttsd_xmltransformerplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkttsd_xmltransformerplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = kttsd_xmltransformerplugin.desktop
servicesdir = $(kde_servicesdir)

# Install data files.
xmltransformerdatadir = $(kde_datadir)/kttsd/xmltransformer/
xmltransformerdata_DATA = xhtml2ssml.xsl xhtml2ssml_simple.xsl

noinst_HEADERS =  xmltransformerconfwidget.h
