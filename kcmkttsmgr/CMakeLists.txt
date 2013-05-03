
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=2404)


########### next target ###############

set(kcm_kttsd_PART_SRCS 
   kcmkttsmgr.cpp 
   kttsjobmgr.cpp
   addtalker.cpp
   talkerwidget.cpp)


qt4_add_dbus_interfaces(kcm_kttsd_PART_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml )

kde4_add_ui_files(kcm_kttsd_PART_SRCS kcmkttsmgrwidget.ui kttsjobmgr.ui talkerwidget.ui )

kde4_add_plugin(kcm_kttsd ${kcm_kttsd_PART_SRCS})


target_link_libraries(kcm_kttsd  ${KDE4_KPARTS_LIBS} ${SPEECHD_LIBRARIES} kttsd  )

install(TARGETS kcm_kttsd  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmkttsd.desktop  DESTINATION ${SERVICES_INSTALL_DIR}  )
install( FILES kcmkttsd_testmessage.desktop  DESTINATION  ${DATA_INSTALL_DIR}/jovie/ )


