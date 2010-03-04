
include_directories( /usr/local/include ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=2400)

########### next target ###############

set(kttsd_SRCS 
   main.cpp 
   kitty.cpp 
   speaker.cpp 
   appdata.cpp 
   ssmlconvert.cpp 
   filtermgr.cpp 
   talkermgr.cpp 
   kittytrayicon.cpp
) 

qt4_add_dbus_adaptor(kttsd_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml kitty.h Kitty)

kde4_add_executable(kttsd_bin ${kttsd_SRCS})

set_target_properties(kttsd_bin PROPERTIES OUTPUT_NAME kttsd)

target_link_libraries(kttsd_bin
    speechd
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDE3SUPPORT_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS} 
    kttsd )

install(TARGETS kttsd_bin  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES SSMLtoPlainText.xsl  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/xslt/ )
install( FILES kttsd.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kttsmgr.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )

