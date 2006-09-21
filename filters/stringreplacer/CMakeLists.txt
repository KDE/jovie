
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_stringreplacerplugin_PART_SRCS stringreplacerconf.cpp stringreplacerproc.cpp stringreplacerplugin.cpp )

kde4_automoc(${kttsd_stringreplacerplugin_PART_SRCS})

kde4_add_ui_files(kttsd_stringreplacerplugin_PART_SRCS stringreplacerconfwidget.ui editreplacementwidget.ui )

kde4_add_plugin(kttsd_stringreplacerplugin WITH_PREFIX ${kttsd_stringreplacerplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_stringreplacerplugin )

target_link_libraries(kttsd_stringreplacerplugin  ${KDE4_KDECORE_LIBS} kttsd )

install(TARGETS kttsd_stringreplacerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install(FILES 
    abbreviations.xml
    chat.xml
    chat-de.xml
    emoticons.xml
    polish_festival_fixes.xml
    polish_festival_unspeakables.xml
    kmail.xml
    festival_unspeakable_chars.xml
    qt2plaintext.xml DESTINATION ${DATA_INSTALL_DIR}/kttsd/stringreplacer/)
    
install( FILES kttsd_stringreplacerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


