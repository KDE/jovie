kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kate/interfaces ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(ktexteditor_kttsd_PART_SRCS katekttsd.cpp )

kde4_automoc(${ktexteditor_kttsd_PART_SRCS})

kde4_add_plugin(ktexteditor_kttsd ${ktexteditor_kttsd_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} ktexteditor_kttsd )

target_link_libraries(ktexteditor_kttsd  ${QT_AND_KDECORE_LIBS} ktexteditor )

install_targets(${LIB_INSTALL_DIR}/kde4 ktexteditor_kttsd )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES ktexteditor_kttsd.desktop )
install_files( ${DATA_INSTALL_DIR}/ktexteditor_kttsd FILES ktexteditor_kttsdui.rc )

kde4_footer()



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
