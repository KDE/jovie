# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = -I$(kde_includes)/arts $(all_includes)
AM_CPPFLAGS = -I$(kde_includes)/arts $(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

##################################################
# check to see if GStreamer is available
##################################################
# gstreamer turned off.  Doesn't work with wave files anyway.
# if link_lib_GST
# gstlibs = -lkdegst -lkdegstplay
# endif
##################################################

#########################################################################
# LIBRARY SECTION
#########################################################################
# This is the library that gets installed.  It's name is used for all
# of the other Makefile.am variables.
lib_LTLIBRARIES = libkttsd.la

# The source, library search path, and link libraries.
libkttsd_la_SOURCES = \
        pluginproc.cpp \
        pluginconf.cpp \
        artsplayer.cpp \
        gstreamerplayer.cpp
libkttsd_la_LDFLAGS = -avoid-version $(all_libraries) -no-undefined
libkttsd_la_LIBADD = $(gstlibs) $(LIB_KIO) -lartskde

# This library is installed as a service.
servicetypes_DATA = kttsd.desktop
servicetypesdir = $(kde_servicetypesdir)
