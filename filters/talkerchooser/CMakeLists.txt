kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kttsd_talkerchooserplugin_PART_SRCS talkerchooserconf.cpp talkerchooserproc.cpp talkerchooserplugin.cpp )

kde4_automoc(${kttsd_talkerchooserplugin_PART_SRCS})

kde4_add_ui3_files(kttsd_talkerchooserplugin_PART_SRCS talkerchooserconfwidget.ui )

kde4_add_plugin(kttsd_talkerchooserplugin WITH_PREFIX ${kttsd_talkerchooserplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_talkerchooserplugin )

target_link_libraries(kttsd_talkerchooserplugin  ${KDE4_KDECORE_LIBS} kttsd )

install_targets(${PLUGIN_INSTALL_DIR} kttsd_talkerchooserplugin )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kttsd/talkerchooser/ FILES female_notifications_rc )
install_files( ${SERVICES_INSTALL_DIR} FILES kttsd_talkerchooserplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#	$(all_includes)
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkttsd_talkerchooserplugin.la
#
#libkttsd_talkerchooserplugin_la_SOURCES = \
#	talkerchooserconfwidget.ui \
#	talkerchooserconf.cpp \
#	talkerchooserproc.cpp \
#	talkerchooserplugin.cpp
#libkttsd_talkerchooserplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
#libkttsd_talkerchooserplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la
#
#services_DATA = kttsd_talkerchooserplugin.desktop
#servicesdir = $(kde_servicesdir)
#
## Install data files.
#talkerchooserdatadir = $(kde_datadir)/kttsd/talkerchooser/
#talkerchooserdata_DATA = \
#	female_notifications_rc
#
#noinst_HEADERS =  talkerchooserconfwidget.h
