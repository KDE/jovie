
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_alsaplugin_PART_SRCS alsaplugin.cpp alsaplayer.cpp )

kde4_automoc(kttsd_alsaplugin ${kttsd_alsaplugin_PART_SRCS})

kde4_add_plugin(kttsd_alsaplugin WITH_PREFIX ${kttsd_alsaplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_alsaplugin )

target_link_libraries(kttsd_alsaplugin  ${KDE4_KDECORE_LIBS} ${ASOUND_LIBRARY} kttsd )

install(TARGETS kttsd_alsaplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kttsd_alsaplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

