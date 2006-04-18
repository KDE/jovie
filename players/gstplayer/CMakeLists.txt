kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kttsd_gstplugin_PART_SRCS gstplugin.cpp gstreamerplayer.cpp )

kde4_automoc(${kttsd_gstplugin_PART_SRCS})

kde4_add_plugin(kttsd_gstplugin WITH_PREFIX ${kttsd_gstplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_gstplugin )

target_link_libraries(kttsd_gstplugin  ${KDE4_KDECORE_LIBS} kttsd )

install_targets(${PLUGIN_INSTALL_DIR} kttsd_gstplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kttsd_gstplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

## Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
## so keep them synchronized.
#INCLUDES = \
#	$(INCLUDES_GST) \
#	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#	$(all_includes)
#
## Let automoc handle all of the metsource files (moc).
#METASOURCES = AUTO
#
##########################################################################
## LIBRARY SECTION
##########################################################################
## This is the library that gets installed.  It's name is used for all
## of the other Makefile.am variables.
#kde_module_LTLIBRARIES = libkttsd_gstplugin.la
#
#libkttsd_gstplugin_la_SOURCES = \
#	gstplugin.cpp \
#	gstreamerplayer.cpp
#libkttsd_gstplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(LDFLAGS_GST) -no-undefined
#libkttsd_gstplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la $(LDADD_GST)
#
## Header files that should not be installed.
#noinst_HEADERS = \
#	gstreamerplayer.h
#
## This library is installed as a plugin.
#services_DATA = kttsd_gstplugin.desktop
#servicesdir = $(kde_servicesdir)
