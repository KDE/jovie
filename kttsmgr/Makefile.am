# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = \
	-I$(top_srcdir)/kttsd/libktts -I$(top_builddir)/kttsd/libktts \
	$(all_includes)
AM_CPPFLAGS = \
	-I$(top_srcdir)/kttsd/libktts -I$(top_builddir)/kttsd/libktts \
	$(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# This is the program that gets installed.  It's name is used for all
# of the other Makefile.am variables.
bin_PROGRAMS = kttsmgr

# The source, library search path, and link libraries.
kttsmgr_SOURCES =  kttsmgr.cpp
kttsmgr_LDFLAGS = $(all_libraries)
kttsmgr_LDADD = $(top_builddir)/kttsd/libktts/libktts.la $(LIB_KDECORE) $(LIB_KUTILS) $(LIB_KDEUI)

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

