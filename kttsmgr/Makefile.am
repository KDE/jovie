# Include paths.
INCLUDES = \
	$(KTTS_KSPEECH_INCLUDE) \
	$(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# This is the program that gets installed.  It's name is used for all
# of the other Makefile.am variables.
bin_PROGRAMS = kttsmgr

kspeech_DIR = $(KTTS_KSPEECH_DIR)

# The source, library search path, and link libraries.
kttsmgr_SOURCES =  \
	kspeech.stub \
	kttsmgr.cpp

kttsmgr_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kttsmgr_LDADD = \
	$(LIB_KUTILS) \
	$(LIB_KDEUI)

# Header files that should not be installed.
noinst_HEADERS = kttsmgr.h

# K Menu entry.
xdg_apps_DATA = kttsmgr.desktop

# Translation message files.
messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kttsmgr.pot

