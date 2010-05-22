
include_directories( ${CMAKE_SOURCE_DIR}/kitty/libkttsd  )


########### next target ###############

set(kitty_xmltransformerplugin_PART_SRCS xmltransformerconf.cpp xmltransformerproc.cpp xmltransformerplugin.cpp )

kde4_add_ui_files(kitty_xmltransformerplugin_PART_SRCS xmltransformerconfwidget.ui )

kde4_add_plugin(kitty_xmltransformerplugin WITH_PREFIX ${kitty_xmltransformerplugin_PART_SRCS})



target_link_libraries(kitty_xmltransformerplugin  ${KDE4_KDECORE_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kttsd )

install(TARGETS kitty_xmltransformerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES xhtml2ssml.xsl xhtml2ssml_simple.xsl  DESTINATION  ${DATA_INSTALL_DIR}/kitty/xmltransformer/ )
install( FILES kitty_xmltransformerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


