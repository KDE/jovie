kde4_header()

macro_optional_find_package(Akode)
if(AKODE_FOUND)
	add_subdirectory(akodeplayer)
endif(AKODE_FOUND)		

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(artsplayer_DIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(alsaplayer_DIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(gstplayer_DIR)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES kttsd_audioplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

##kdevelop: artsplayer_DIR = artsplayer
#if include_kttsd_artsplayer
#    artsplayer_DIR = artsplayer
#endif
#
##kdevelop: gstplayer_DIR = gstplayer
#if include_kttsd_gstplayer
#    gstplayer_DIR = gstplayer
#endif
#
##kdevelop: alsaplayer_DIR = alsaplayer
#if include_kttsd_alsaplayer
#    alsaplayer_DIR = alsaplayer
#endif
#
##kdevelop: akodeplayer_DIR = akodeplayer
#if include_kttsd_akodeplayer
#    akodeplayer_DIR = akodeplayer
#endif
#
#SUBDIRS = \
#    $(artsplayer_DIR) \
#    $(alsaplayer_DIR) \
#    $(gstplayer_DIR) \
#    $(akodeplayer_DIR)
#
#kde_servicetypes_DATA = kttsd_audioplugin.desktop
