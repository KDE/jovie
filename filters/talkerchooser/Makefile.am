INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	$(all_includes)

METASOURCES = AUTO

kde_module_LTLIBRARIES = libkttsd_talkerchooserplugin.la

libkttsd_talkerchooserplugin_la_SOURCES = \
	talkerchooserconfwidget.ui \
	talkerchooserconf.cpp \
	talkerchooserproc.cpp \
	talkerchooserplugin.cpp
libkttsd_talkerchooserplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkttsd_talkerchooserplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la

services_DATA = kttsd_talkerchooserplugin.desktop
servicesdir = $(kde_servicesdir)

# Install data files.
talkerchooserdatadir = $(kde_datadir)/kttsd/talkerchooser/
talkerchooserdata_DATA = \
	female_notifications_rc

noinst_HEADERS =  talkerchooserconfwidget.h
