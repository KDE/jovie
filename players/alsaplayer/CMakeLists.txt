kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kttsd_alsaplugin_PART_SRCS alsaplugin.cpp alsaplayer.cpp )

kde4_automoc(${kttsd_alsaplugin_PART_SRCS})

kde4_add_plugin(kttsd_alsaplugin WITH_PREFIX ${kttsd_alsaplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_alsaplugin )

target_link_libraries(kttsd_alsaplugin  ${KDE4_KDECORE_LIBS} kttsd )

install_targets(${LIB_INSTALL_DIR}/kde4 kttsd_alsaplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kttsd_alsaplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

## Include paths.
#INCLUDES = \
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
#kde_module_LTLIBRARIES = libkttsd_alsaplugin.la
#
#libkttsd_alsaplugin_la_SOURCES = \
#    alsaplugin.cpp \
#	alsaplayer.cpp
#libkttsd_alsaplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(LIBASOUND) -no-undefined
#libkttsd_alsaplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la
#
## Header files that should not be installed.
#noinst_HEADERS = \
#	alsaplayer.h \
#	formats.h
#
## This library is installed as a plugin.
#services_DATA = kttsd_alsaplugin.desktop
#servicesdir = $(kde_servicesdir)
