
include_directories( ${CMAKE_SOURCE_DIR}/kitty/libkttsd  )


########### next target ###############

set(kitty_stringreplacerplugin_PART_SRCS stringreplacerconf.cpp stringreplacerproc.cpp stringreplacerplugin.cpp )

kde4_add_ui_files(kitty_stringreplacerplugin_PART_SRCS stringreplacerconfwidget.ui editreplacementwidget.ui )

kde4_add_plugin(kitty_stringreplacerplugin WITH_PREFIX ${kitty_stringreplacerplugin_PART_SRCS})



target_link_libraries(kitty_stringreplacerplugin  ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} kttsd )

install(TARGETS kitty_stringreplacerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


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
    qt2plaintext.xml DESTINATION ${DATA_INSTALL_DIR}/kitty/stringreplacer/)

install( FILES kitty_stringreplacerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


