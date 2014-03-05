

########### next target ###############

set(jovie_stringreplacerplugin_PART_SRCS
    selectlanguagedlg.cpp
    stringreplacerconf.cpp 
    stringreplacerproc.cpp
    stringreplacerplugin.cpp 
    cdataescaper.cpp)

kde4_add_ui_files(jovie_stringreplacerplugin_PART_SRCS stringreplacerconfwidget.ui editreplacementwidget.ui )

kde4_add_plugin(jovie_stringreplacerplugin ${jovie_stringreplacerplugin_PART_SRCS})



target_link_libraries(jovie_stringreplacerplugin  ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} kttsd )

install(TARGETS jovie_stringreplacerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### test cdata escaper ##########

set(test_cdataescaper_SRCS testcdataescaper.cpp cdataescaper.cpp)
kde4_add_unit_test(
    test_cdataescaper TESTNAME jovie-cdata_esecaper
    ${test_cdataescaper_SRCS}
)
target_link_libraries(test_cdataescaper
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
)

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
    qt2plaintext.xml DESTINATION ${DATA_INSTALL_DIR}/jovie/stringreplacer/)

install( FILES jovie_stringreplacerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


