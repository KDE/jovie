
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_fliteplugin_PART_SRCS fliteconf.cpp fliteproc.cpp fliteplugin.cpp )

kde4_automoc(${kttsd_fliteplugin_PART_SRCS})

kde4_add_ui_files(kttsd_fliteplugin_PART_SRCS fliteconfwidget.ui )

kde4_add_plugin(kttsd_fliteplugin WITH_PREFIX ${kttsd_fliteplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_fliteplugin )

target_link_libraries(kttsd_fliteplugin  ${KDE4_KDECORE_LIBS} kttsd )

install(TARGETS kttsd_fliteplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kttsd_fliteplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#	$(all_includes)
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkttsd_fliteplugin.la
#
#libkttsd_fliteplugin_la_SOURCES = \
#        fliteconfwidget.ui \
#        fliteconf.cpp \
#        fliteproc.cpp \
#        fliteplugin.cpp 
#libkttsd_fliteplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
#libkttsd_fliteplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la
#
#services_DATA = kttsd_fliteplugin.desktop
#servicesdir = $(kde_servicesdir)
#
#noinst_HEADERS =  fliteproc.h fliteconf.h fliteconfwidget.h
