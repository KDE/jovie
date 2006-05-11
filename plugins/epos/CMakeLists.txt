kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_eposplugin_PART_SRCS eposconf.cpp eposproc.cpp eposplugin.cpp )

kde4_automoc(${kttsd_eposplugin_PART_SRCS})

kde4_add_ui3_files(kttsd_eposplugin_PART_SRCS eposconfwidget.ui )

kde4_add_plugin(kttsd_eposplugin WITH_PREFIX ${kttsd_eposplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_eposplugin )

target_link_libraries(kttsd_eposplugin  ${KDE4_KDECORE_LIBS} kttsd )

install_targets(${PLUGIN_INSTALL_DIR} kttsd_eposplugin )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kttsd_eposplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = \
#        -I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#        $(all_includes)
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkttsd_eposplugin.la
#
#libkttsd_eposplugin_la_SOURCES = \
#        eposconfwidget.ui \
#        eposconf.cpp \
#        eposproc.cpp \
#        eposplugin.cpp 
#libkttsd_eposplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
#libkttsd_eposplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la
#
#services_DATA = kttsd_eposplugin.desktop
#servicesdir = $(kde_servicesdir)
#
#noinst_HEADERS =  eposproc.h eposconf.h eposconfwidget.h
