
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=2401)

set(kttsmgr_SRCS 
    kttsmgr.cpp )

qt4_add_dbus_interfaces(kttsmgr_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml )

kde4_add_executable(kttsmgr ${kttsmgr_SRCS})

target_link_libraries(kttsmgr
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS} )

install(TARGETS kttsmgr  ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES kttsmgr.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
