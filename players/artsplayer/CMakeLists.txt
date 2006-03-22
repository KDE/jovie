kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kttsd_artsplugin_PART_SRCS artsplugin.cpp artsplayer.cpp )

kde4_automoc(${kttsd_artsplugin_PART_SRCS})

kde4_add_plugin(kttsd_artsplugin WITH_PREFIX ${kttsd_artsplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_artsplugin )

target_link_libraries(kttsd_artsplugin  ${KDE4_KDECORE_LIBS} kttsd artskde )

install_targets(${LIB_INSTALL_DIR}/kde4 kttsd_artsplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kttsd_artsplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

## Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
## so keep them synchronized.
#INCLUDES = \
#	-I$(kde_includes)/arts \
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
#kde_module_LTLIBRARIES = libkttsd_artsplugin.la
#
#libkttsd_artsplugin_la_SOURCES = \
#    artsplugin.cpp \
#	artsplayer.cpp
#libkttsd_artsplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) -no-undefined
#libkttsd_artsplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la -lartskde
#
## Header files that should not be installed.
#noinst_HEADERS = \
#	artsplayer.h
#
## This library is installed as a plugin.
#services_DATA = kttsd_artsplugin.desktop
#servicesdir = $(kde_servicesdir)
