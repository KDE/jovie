
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_hadifixplugin_PART_SRCS hadifixconf.cpp hadifixproc.cpp hadifixplugin.cpp )

kde4_automoc(${kttsd_hadifixplugin_PART_SRCS})

kde4_add_ui_files(kttsd_hadifixplugin_PART_SRCS hadifixconfigui.ui voicefileui.ui )

kde4_add_plugin(kttsd_hadifixplugin WITH_PREFIX ${kttsd_hadifixplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_hadifixplugin )

target_link_libraries(kttsd_hadifixplugin  ${KDE4_KDECORE_LIBS} kttsd )

install(TARGETS kttsd_hadifixplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES SSMLtoTxt2pho.xsl  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/hadifix/xslt/ )
install( FILES kttsd_hadifixplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
