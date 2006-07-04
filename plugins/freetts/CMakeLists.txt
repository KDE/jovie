
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_freettsplugin_PART_SRCS freettsconf.cpp freettsproc.cpp freettsplugin.cpp )

kde4_automoc(${kttsd_freettsplugin_PART_SRCS})

kde4_add_ui_files(kttsd_freettsplugin_PART_SRCS freettsconfigwidget.ui )

kde4_add_plugin(kttsd_freettsplugin WITH_PREFIX ${kttsd_freettsplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_freettsplugin )

target_link_libraries(kttsd_freettsplugin  ${KDE4_KDECORE_LIBS} kttsd )

install(TARGETS kttsd_freettsplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kttsd_freettsplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




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
