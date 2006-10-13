



########### next target ###############

set(kttsmgr_SRCS 
    kttsmgr.cpp )

qt4_add_dbus_interfaces(kttsmgr_SRCS org.kde.KSpeech.xml )

kde4_automoc(kttsmgr ${kttsmgr_SRCS})

kde4_add_executable(kttsmgr ${kttsmgr_SRCS})

target_link_libraries(kttsmgr
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS} )

install(TARGETS kttsmgr  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kttsmgr.desktop  DESTINATION  ${XDG_APPS_DIR} )
