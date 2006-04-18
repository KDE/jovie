kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kttsd_sbdplugin_PART_SRCS sbdproc.cpp sbdconf.cpp sbdplugin.cpp )

kde4_automoc(${kttsd_sbdplugin_PART_SRCS})

kde4_add_ui3_files(kttsd_sbdplugin_PART_SRCS sbdconfwidget.ui )

kde4_add_plugin(kttsd_sbdplugin WITH_PREFIX ${kttsd_sbdplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_sbdplugin )

target_link_libraries(kttsd_sbdplugin  ${KDE4_KDECORE_LIBS} kttsd )

install_targets(${PLUGIN_INSTALL_DIR} kttsd_sbdplugin )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kttsd/sbd/ FILES standard_sbdrc  	polish_festival_sbdrc )
install_files( ${SERVICES_INSTALL_DIR} FILES kttsd_sbdplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#	$(all_includes)
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkttsd_sbdplugin.la
#
#libkttsd_sbdplugin_la_SOURCES = \
#	sbdproc.cpp \
#	sbdconf.cpp \
#	sbdconfwidget.ui \
#	sbdplugin.cpp
#
#libkttsd_sbdplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
#libkttsd_sbdplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la
#
#services_DATA = kttsd_sbdplugin.desktop
#servicesdir = $(kde_servicesdir)
#
## Install data files.
#sbddatadir = $(kde_datadir)/kttsd/sbd/
#sbddata_DATA = \
#	standard_sbdrc \
#	polish_festival_sbdrc
#
#noinst_HEADERS = sbdproc.h
