# Include paths.
INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	$(CFLAGS_GST) \
	$(KTTS_KSPEECH_INCLUDE) \
	$(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# KCMODULE SECTION
#########################################################################
# This is the kcmodule that gets installed.  It's name is used for all
# of the other Makefile.am variables.

kde_module_LTLIBRARIES = kcm_kttsd.la

kspeech_DIR = $(KTTS_KSPEECH_DIR)
kspeechsink_DIR = $(KTTS_KSPEECH_DIR)

# The source, library search path, and link libraries.
kcm_kttsd_la_SOURCES = \
	kspeech.stub kspeechsink.skel \
	kcmkttsmgr.cpp \
	kcmkttsmgrwidget.ui \
	addtalker.cpp \
	addtalkerwidget.ui
kcm_kttsd_la_LDFLAGS = -avoid-version -module -no-undefined $(all_libraries)
kcm_kttsd_la_LIBADD = \
	$(top_builddir)/kttsd/libkttsd/libkttsd.la \
	$(LIB_KIO) 

# Header files that should not be installed.
noinst_HEADERS = \
	kcmkttsmgr.h \
	kcmkttsmgrwidget.h \
	kcmkttsmgrwidget.ui.h \
	addtalker.h

# K Menu entry.
xdg_apps_DATA = kcmkttsd.desktop

# Install data files.
kcmkttsddatadir = $(kde_datadir)/kttsd/
kcmkttsddata_DATA = kcmkttsd_testmessage.desktop
