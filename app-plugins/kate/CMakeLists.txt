
include_directories( ${CMAKE_SOURCE_DIR}/kate/interfaces  )


########### next target ###############

set(ktexteditor_kttsd_PART_SRCS katekttsd.cpp )

kde4_add_plugin(ktexteditor_kttsd ${ktexteditor_kttsd_PART_SRCS})



target_link_libraries(ktexteditor_kttsd  ${KDE4_KTEXTEDITOR_LIBS} )

install(TARGETS ktexteditor_kttsd  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ktexteditor_kttsd.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES ktexteditor_kttsdui.rc  DESTINATION  ${DATA_INSTALL_DIR}/ktexteditor_kttsd )




#original Makefile.am contents follow:

## Allow build from within kdebase or kdeaccessibility.
#INCLUDES    = \
#	-I$(top_srcdir)/kate/interfaces \
#	-I$(kde_includes)/kate \
#	$(all_includes)
#
#METASOURCES = AUTO
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = ktexteditor_kttsd.la
#
#ktexteditor_kttsd_la_SOURCES = katekttsd.cpp
#ktexteditor_kttsd_la_LIBADD  = -lktexteditor
#ktexteditor_kttsd_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#
#kttsddatadir = $(kde_datadir)/ktexteditor_kttsd
#kttsddata_DATA = ktexteditor_kttsdui.rc
#
#kde_services_DATA = ktexteditor_kttsd.desktop
