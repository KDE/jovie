
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_xmltransformerplugin_PART_SRCS xmltransformerconf.cpp xmltransformerproc.cpp xmltransformerplugin.cpp )

kde4_automoc(${kttsd_xmltransformerplugin_PART_SRCS})

kde4_add_ui_files(kttsd_xmltransformerplugin_PART_SRCS xmltransformerconfwidget.ui )

kde4_add_plugin(kttsd_xmltransformerplugin WITH_PREFIX ${kttsd_xmltransformerplugin_PART_SRCS})



target_link_libraries(kttsd_xmltransformerplugin  ${KDE4_KDECORE_LIBS} kttsd )

install(TARGETS kttsd_xmltransformerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES xhtml2ssml.xsl xhtml2ssml_simple.xsl  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/xmltransformer/ )
install( FILES kttsd_xmltransformerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


