
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${CMAKE_BINARY_DIR}/kttsd/libkttsd )


########### next target ###############

set(kttsd_talkerchooserplugin_PART_SRCS talkerchooserconf.cpp talkerchooserproc.cpp talkerchooserplugin.cpp )


kde4_add_ui_files(kttsd_talkerchooserplugin_PART_SRCS talkerchooserconfwidget.ui )


kde4_add_plugin(kttsd_talkerchooserplugin WITH_PREFIX ${kttsd_talkerchooserplugin_PART_SRCS})



target_link_libraries(kttsd_talkerchooserplugin  ${KDE4_KDECORE_LIBS} kttsd )

install(TARGETS kttsd_talkerchooserplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES female_notifications_rc  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/talkerchooser/ )
install( FILES kttsd_talkerchooserplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

