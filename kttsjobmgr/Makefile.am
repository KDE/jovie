# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
	$(KTTS_KSPEECH_INCLUDE) \
	$(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# KPART SECTION
#########################################################################
# This is the kpart that gets installed.  It's name is used for all
# of the other Makefile.am variables.
kde_module_LTLIBRARIES =  libkttsjobmgrpart.la

kspeech_DIR = $(KTTS_KSPEECH_DIR)
kspeechsink_DIR = $(KTTS_KSPEECH_DIR)

# The source, library search path, and link libraries.
libkttsjobmgrpart_la_SOURCES = \
	kspeech.stub kspeechsink.skel \
	kttsjobmgr.cpp \
	selecttalkerwidget.ui
libkttsjobmgrpart_la_LDFLAGS = -avoid-version -module -no-undefined $(all_libraries) $(KDE_PLUGIN)
libkttsjobmgrpart_la_LIBADD = \
	$(top_builddir)/kttsd/libkttsd/libkttsd.la \
	$(LIB_KFILE) $(LIBVM) $(LIB_KPARTS)

# Header files not to be installed.
noinst_HEADERS = selecttalkerwidget.h

# Install desktop file to standard services directory.
kde_services_DATA =  kttsjobmgr.desktop

# This rc file should never have been installed.
# createGUI not supported within a KCModule.
install-data-local:
	rm -f $(DESTDIR)$(kde_datadir)/kttsjobmgr/kttsjobmgrui.rc
