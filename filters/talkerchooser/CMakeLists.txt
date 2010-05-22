
include_directories( ${CMAKE_SOURCE_DIR}/kitty/libkttsd ${CMAKE_BINARY_DIR}/kitty/libkttsd )


########### next target ###############

set(kitty_talkerchooserplugin_PART_SRCS talkerchooserconf.cpp talkerchooserproc.cpp talkerchooserplugin.cpp )


kde4_add_ui_files(kitty_talkerchooserplugin_PART_SRCS talkerchooserconfwidget.ui )


kde4_add_plugin(kitty_talkerchooserplugin WITH_PREFIX ${kitty_talkerchooserplugin_PART_SRCS})



target_link_libraries(kitty_talkerchooserplugin  ${KDE4_KIO_LIBS} kttsd )

install(TARGETS kitty_talkerchooserplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES female_notifications_rc  DESTINATION  ${DATA_INSTALL_DIR}/kitty/talkerchooser/ )
install( FILES kitty_talkerchooserplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

