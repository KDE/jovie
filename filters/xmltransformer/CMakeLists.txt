

########### next target ###############

set(jovie_xmltransformerplugin_PART_SRCS xmltransformerconf.cpp xmltransformerproc.cpp xmltransformerplugin.cpp )

kde4_add_ui_files(jovie_xmltransformerplugin_PART_SRCS xmltransformerconfwidget.ui )

kde4_add_plugin(jovie_xmltransformerplugin ${jovie_xmltransformerplugin_PART_SRCS})



target_link_libraries(jovie_xmltransformerplugin  ${KDE4_KIO_LIBS} kttsd )

install(TARGETS jovie_xmltransformerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES xhtml2ssml.xsl xhtml2ssml_simple.xsl  DESTINATION  ${DATA_INSTALL_DIR}/jovie/xmltransformer/ )
install( FILES jovie_xmltransformerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


