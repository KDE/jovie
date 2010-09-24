
include_directories( ${CMAKE_SOURCE_DIR}/jovie/libkttsd ${CMAKE_BINARY_DIR}/jovie/libkttsd )


########### next target ###############

set(jovie_talkerchooserplugin_PART_SRCS talkerchooserconf.cpp talkerchooserproc.cpp talkerchooserplugin.cpp )


kde4_add_ui_files(jovie_talkerchooserplugin_PART_SRCS talkerchooserconfwidget.ui )


kde4_add_plugin(jovie_talkerchooserplugin ${jovie_talkerchooserplugin_PART_SRCS})



target_link_libraries(jovie_talkerchooserplugin  ${KDE4_KIO_LIBS} kttsd )

install(TARGETS jovie_talkerchooserplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES female_notifications_rc  DESTINATION  ${DATA_INSTALL_DIR}/jovie/talkerchooser/ )
install( FILES jovie_talkerchooserplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

