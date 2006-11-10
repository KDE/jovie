
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_festivalintplugin_PART_SRCS festivalintconf.cpp festivalintproc.cpp festivalintplugin.cpp )

kde4_automoc(${kttsd_festivalintplugin_PART_SRCS})

kde4_add_ui_files(kttsd_festivalintplugin_PART_SRCS festivalintconfwidget.ui )

kde4_add_plugin(kttsd_festivalintplugin WITH_PREFIX ${kttsd_festivalintplugin_PART_SRCS})



target_link_libraries(kttsd_festivalintplugin  ${KDE4_KDECORE_LIBS} kttsd )

install(TARGETS kttsd_festivalintplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES voices sabletowave.scm  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/festivalint/ )
install( FILES SSMLtoSable.xsl  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/festivalint/xslt/ )
install( FILES kttsd_festivalintplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

