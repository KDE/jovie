# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = $(all_includes)
AM_CPPFLAGS = $(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# LIBRARY SECTION
#########################################################################
# This is the library that gets installed.  It's name is used for all
# of the other Makefile.am variables.
lib_LTLIBRARIES = libkttsd.la

# The source, library search path, and link libraries.
libkttsd_la_SOURCES = pluginproc.cpp 	pluginconf.cpp 	testplayer.cpp \
		stretcher.cpp 	talkercode.cpp 	filterproc.cpp 	filterconf.cpp 	sbdfilterproc.cpp \
	utils.cpp

libkttsd_la_LDFLAGS = -version-info 1:0:0 $(all_libraries) -no-undefined
libkttsd_la_LIBADD = $(LIB_KIO)

# Header files that should not be installed.
noinst_HEADERS = pluginproc.h 	pluginconf.h 	player.h 	testplayer.h \
		stretcher.h 	talkercode.h utils.h

# This library is installed as a service.
servicetypes_DATA = kttsd_synthplugin.desktop
servicetypesdir = $(kde_servicetypesdir)
