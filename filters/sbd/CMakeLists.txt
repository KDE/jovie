
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_sbdplugin_PART_SRCS sbdproc.cpp sbdconf.cpp sbdplugin.cpp )

kde4_add_ui_files(kttsd_sbdplugin_PART_SRCS sbdconfwidget.ui )

kde4_add_plugin(kttsd_sbdplugin WITH_PREFIX ${kttsd_sbdplugin_PART_SRCS})



target_link_libraries(kttsd_sbdplugin  ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} kttsd )

install(TARGETS kttsd_sbdplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES standard_sbdrc  	polish_festival_sbdrc  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/sbd/ )
install( FILES kttsd_sbdplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

