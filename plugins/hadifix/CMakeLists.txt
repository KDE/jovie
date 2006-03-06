kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kttsd_hadifixplugin_PART_SRCS hadifixconf.cpp hadifixproc.cpp hadifixplugin.cpp )

kde4_automoc(${kttsd_hadifixplugin_PART_SRCS})

kde4_add_ui3_files(kttsd_hadifixplugin_PART_SRCS hadifixconfigui.ui voicefileui.ui )

kde4_add_plugin(kttsd_hadifixplugin WITH_PREFIX ${kttsd_hadifixplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_hadifixplugin )

target_link_libraries(kttsd_hadifixplugin  ${QT_AND_KDECORE_LIBS} kttsd )

install_targets(${LIB_INSTALL_DIR}/kde4 kttsd_hadifixplugin )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kttsd/hadifix/xslt/ FILES SSMLtoTxt2pho.xsl )
install_files( ${SERVICES_INSTALL_DIR} FILES kttsd_hadifixplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#	$(all_includes)
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkttsd_hadifixplugin.la
#
#libkttsd_hadifixplugin_la_SOURCES = \
#	hadifixconf.cpp \
#	hadifixproc.cpp \
#	hadifixplugin.cpp \
#	hadifixconfigui.ui \
#	voicefileui.ui
#libkttsd_hadifixplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
#libkttsd_hadifixplugin_la_LIBADD = $(LIB_KDECORE) $(LIB_KFILE) $(top_builddir)/kttsd/libkttsd/libkttsd.la
#
#services_DATA = kttsd_hadifixplugin.desktop
#servicesdir = $(kde_servicesdir)
#
## Data files.
#festivalintxsltdatadir = $(kde_datadir)/kttsd/hadifix/xslt/
#festivalintxsltdata_DATA = SSMLtoTxt2pho.xsl
