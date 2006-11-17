set(kspeak_SRCS 
    main.cpp )

# qt4_add_dbus_interfaces(kttsmgr_SRCS org.kde.KSpeech.xml )

# kde4_automoc(${kttsmgr_SRCS})

kde4_add_executable(kspeak ${kspeak_SRCS})

target_link_libraries(kspeak
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS} )

# install(TARGETS kspeak DESTINATION ${BIN_INSTALL_DIR} )
