# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = \
	-I$(top_srcdir)/kttsd/libkttsd \
	-I$(kde_includes)/arts \
	$(CFLAGS_GST) \
	$(all_includes)

# Let am_edit/unsermake handle all of the metasource files (moc).
METASOURCES = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# This is the program that gets installed.  It's name is used for all
# of the other Makefile.am variables.
bin_PROGRAMS = kttsd

kspeech_DIR = $(kde_includes)
kspeechsink_DIR = $(kde_includes)

# The source, library search path, and link libraries.
# Note: .skel files cause DCOPIDL compiler to generate _skel.cpp file and compile it.
kttsd_SOURCES = \
    kspeech.skel kspeechsink.stub\
	main.cpp \
	kttsd.cpp \
	speaker.cpp \
	speechdata.cpp \
	kttsd.skel \
	threadedplugin.cpp \
	ssmlconvert.cpp \
	filtermgr.cpp \
	talkermgr.cpp

kttsd_LDFLAGS = -avoid-version -module $(LDFLAGS_GST) $(all_libraries) $(KDE_RPATH)
kttsd_LDADD = \
	$(top_builddir)/kttsd/libkttsd/libkttsd.la \
	$(LIB_KDECORE) \
	$(LDADD_GST) \
	$(LIB_KIO) \
	$(LIB_KUTILS)

# Install desktop file to standard services directory.
kde_services_DATA = kttsd.desktop

# Install data.
kttsddatadir = $(kde_datadir)/kttsd/xslt/
kttsddata_DATA = SSMLtoPlainText.xsl

# Translation message files.
messages: rc.cpp
	$(XGETTEXT) -nki18n *.cpp -o $(podir)/kttsd.pot

noinst_HEADERS = threadedplugin.h ssmlconvert.h

