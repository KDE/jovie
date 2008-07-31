
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_commandplugin_PART_SRCS commandconf.cpp commandproc.cpp commandplugin.cpp )

kde4_add_ui_files(kttsd_commandplugin_PART_SRCS commandconfwidget.ui )

kde4_add_plugin(kttsd_commandplugin WITH_PREFIX ${kttsd_commandplugin_PART_SRCS})



target_link_libraries(kttsd_commandplugin  ${KDE4_KDECORE_LIBS} kttsd  ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kttsd_commandplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kttsd_commandplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
