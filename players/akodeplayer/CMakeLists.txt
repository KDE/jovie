kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kttsd_akodeplugin_PART_SRCS akodeplugin.cpp akodeplayer.cpp )

kde4_automoc(${kttsd_akodeplugin_PART_SRCS})

kde4_add_plugin(kttsd_akodeplugin WITH_PREFIX ${kttsd_akodeplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_akodeplugin )

target_link_libraries(kttsd_akodeplugin  ${KDE4_KDECORE_LIBS} kttsd ${AKODE_LIBRARIES})

install_targets(${LIB_INSTALL_DIR}/kde4 kttsd_akodeplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kttsd_akodeplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#	$(akode_includes) \
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
#kde_module_LTLIBRARIES = libkttsd_akodeplugin.la
#
#libkttsd_akodeplugin_la_SOURCES = \
#	akodeplugin.cpp \
#	akodeplayer.cpp
#libkttsd_akodeplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) -no-undefined
#libkttsd_akodeplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la $(akode_libs)
#
## Header files that should not be installed.
#noinst_HEADERS = \
#	akodeplayer.h
#
## This library is installed as a plugin.
#services_DATA = kttsd_akodeplugin.desktop
#servicesdir = $(kde_servicesdir)
