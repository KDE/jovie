# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = \
	$(all_includes)
AM_CPPFLAGS = \
	$(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# This is the program that gets installed.  It's name is used for all
# of the other Makefile.am variables.
bin_PROGRAMS = kttsmgr

kspeech_DIR = $(kde_includes)

# The source, library search path, and link libraries.
kttsmgr_SOURCES =  \
	kspeech.stub \
	kttsmgr.cpp

kttsmgr_LDFLAGS = $(all_libraries)
kttsmgr_LDADD = \
	$(LIB_KDECORE) $(LIB_KUTILS) \
	$(LIB_KDEUI)

# Header files that should not be installed.
noinst_HEADERS = kttsmgr.h

# Install desktop file to application directory.
applnkApplicationdir = $(kde_appsdir)/Application
applnkApplication_DATA = kttsmgr.desktop

# K Menu entry.
xdg_apps_DATA = kttsmgr.desktop

# Translation message files.
messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kttsmgr.pot

