
include_directories( ${CMAKE_SOURCE_DIR}/kate/interfaces  )


########### next target ###############

set(ktexteditor_kttsd_PART_SRCS katekttsd.cpp )

kde4_add_plugin(ktexteditor_kttsd ${ktexteditor_kttsd_PART_SRCS})



target_link_libraries(ktexteditor_kttsd  ${KDE4_KTEXTEDITOR_LIBS} )

install(TARGETS ktexteditor_kttsd  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ktexteditor_kttsd.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES ktexteditor_kttsdui.rc  DESTINATION  ${DATA_INSTALL_DIR}/ktexteditor_kttsd )
