# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = \
	-I$(top_srcdir)/kttsd/libktts \
	-I$(top_srcdir)/kttsd/libkttsd \
	-I$(kde_includes)/arts \
	-I$(CFLAGS_GST) \
	$(all_includes)
AM_CPPFLAGS = \
	-I$(top_srcdir)/kttsd/libkttsd \
	-I$(top_srcdir)/kttsd/libktts \
	-I$(kde_includes)/arts \
	-I$(CFLAGS_GST) \
	$(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# This is the program that gets installed.  It's name is used for all
# of the other Makefile.am variables.
bin_PROGRAMS = kttsd

# The source, library search path, and link libraries.
# Note: .skel files cause DCOPIDL compiler to generate _skel.cpp file and compile it.
kttsd_SOURCES = \
	main.cpp \
	kttsd.cpp \
	speaker.cpp \
	speechdata.cpp \
	kttsd.skel \
	threadedplugin.cpp \
	stretcher.cpp \
	ssmlconvert.cpp

kttsd_LDFLAGS = -avoid-version -module $(LDFLAGS_GST) $(all_libraries)
kttsd_LDADD = \
	$(top_builddir)/kttsd/libktts/libktts.la \
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

noinst_HEADERS = threadedplugin.h stretcher.h ssmlconvert.h

