
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_festivalintplugin_PART_SRCS festivalintconf.cpp festivalintproc.cpp festivalintplugin.cpp )

kde4_automoc(${kttsd_festivalintplugin_PART_SRCS})

kde4_add_ui_files(kttsd_festivalintplugin_PART_SRCS festivalintconfwidget.ui )

kde4_add_plugin(kttsd_festivalintplugin WITH_PREFIX ${kttsd_festivalintplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_festivalintplugin )

target_link_libraries(kttsd_festivalintplugin  ${KDE4_KDECORE_LIBS} kttsd )

install(TARGETS kttsd_festivalintplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES voices sabletowave.scm  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/festivalint/ )
install( FILES SSMLtoSable.xsl  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/festivalint/xslt/ )
install( FILES kttsd_festivalintplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#	$(all_includes)
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkttsd_festivalintplugin.la
#
#libkttsd_festivalintplugin_la_SOURCES = \
#	festivalintconfwidget.ui \
#	festivalintconf.cpp \
#	festivalintproc.cpp \
#	festivalintplugin.cpp
#libkttsd_festivalintplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
#libkttsd_festivalintplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la
#
#services_DATA = kttsd_festivalintplugin.desktop
#servicesdir = $(kde_servicesdir)
#
## Install data files.
#festivalintdatadir = $(kde_datadir)/kttsd/festivalint/
#festivalintdata_DATA = voices sabletowave.scm
#
#festivalintxsltdatadir = $(kde_datadir)/kttsd/festivalint/xslt/
#festivalintxsltdata_DATA = SSMLtoSable.xsl
#
#noinst_HEADERS =  festivalintconfwidget.h
