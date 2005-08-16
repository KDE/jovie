# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = \
	$(INCLUDES_GST) \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	$(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# LIBRARY SECTION
#########################################################################
# This is the library that gets installed.  It's name is used for all
# of the other Makefile.am variables.
kde_module_LTLIBRARIES = libkttsd_gstplugin.la

libkttsd_gstplugin_la_SOURCES = \
	gstplugin.cpp \
	gstreamerplayer.cpp
libkttsd_gstplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(LDFLAGS_GST) -no-undefined
libkttsd_gstplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la $(LDADD_GST)

# Header files that should not be installed.
noinst_HEADERS = \
	gstreamerplayer.h

# This library is installed as a plugin.
services_DATA = kttsd_gstplugin.desktop
servicesdir = $(kde_servicesdir)
