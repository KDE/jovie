
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_phononplugin_PART_SRCS phononplugin.cpp phononplayer.cpp )

kde4_automoc(${kttsd_phononplugin_PART_SRCS})

kde4_add_plugin(kttsd_phononplugin WITH_PREFIX ${kttsd_phononplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_phononplugin )

target_link_libraries(kttsd_phononplugin  ${KDE4_KDECORE_LIBS} ${KDE4_PHONONCORE_LIBRARY} kttsd )

install(TARGETS kttsd_phononplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kttsd_phononplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
