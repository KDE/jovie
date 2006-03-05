kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kttsd_commandplugin_PART_SRCS commandconf.cpp commandproc.cpp commandplugin.cpp )

kde4_automoc(${kttsd_commandplugin_PART_SRCS})

kde4_add_ui_files(kttsd_commandplugin_PART_SRCS commandconfwidget.ui )

kde4_add_plugin(kttsd_commandplugin WITH_PREFIX ${kttsd_commandplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_commandplugin )

target_link_libraries(kttsd_commandplugin  ${QT_AND_KDECORE_LIBS} kttsd )

install_targets(${LIB_INSTALL_DIR}/kde4 kttsd_commandplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kttsd_commandplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = \
#        -I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#        $(all_includes)
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkttsd_commandplugin.la
#
#libkttsd_commandplugin_la_SOURCES = \
#        commandconfwidget.ui \
#        commandconf.cpp \
#        commandproc.cpp \
#        commandplugin.cpp
#libkttsd_commandplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
#libkttsd_commandplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la
#
#services_DATA = kttsd_commandplugin.desktop
#servicesdir = $(kde_servicesdir)
