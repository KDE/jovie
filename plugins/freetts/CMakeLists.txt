kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kttsd_freettsplugin_PART_SRCS freettsconf.cpp freettsproc.cpp freettsplugin.cpp )

kde4_automoc(${kttsd_freettsplugin_PART_SRCS})

kde4_add_ui3_files(kttsd_freettsplugin_PART_SRCS freettsconfigwidget.ui )

kde4_add_plugin(kttsd_freettsplugin WITH_PREFIX ${kttsd_freettsplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_freettsplugin )

target_link_libraries(kttsd_freettsplugin  ${KDE4_KDECORE_LIBS} kttsd )

install_targets(${PLUGIN_INSTALL_DIR} kttsd_freettsplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kttsd_freettsplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#	$(all_includes)
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkttsd_freettsplugin.la
#
#libkttsd_freettsplugin_la_SOURCES = \
#				freettsconfigwidget.ui \
#				freettsconf.cpp \
#				freettsproc.cpp \
#				freettsplugin.cpp
#libkttsd_freettsplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
#libkttsd_freettsplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la
#
#services_DATA = kttsd_freettsplugin.desktop
#servicesdir = $(kde_servicesdir)
