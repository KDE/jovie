
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${CMAKE_BINARY_DIR}/kttsd/libkttsd )


########### next target ###############

set(kcm_kttsd_PART_SRCS 
   kcmkttsmgr.cpp 
   addtalker.cpp 
   selectevent.cpp )

kde4_automoc(${kcm_kttsd_PART_SRCS})

qt4_add_dbus_interfaces(kcm_kttsd_PART_SRCS org.kde.KSpeech.xml )

kde4_add_ui_files(kcm_kttsd_PART_SRCS kcmkttsmgrwidget.ui addtalkerwidget.ui selecteventwidget.ui )

kde4_add_plugin(kcm_kttsd ${kcm_kttsd_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kttsd )

target_link_libraries(kcm_kttsd  ${KDE4_KPARTS_LIBS} kttsd  )

install(TARGETS kcm_kttsd  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmkttsd.desktop  DESTINATION ${SERVICES_INSTALL_DIR}  )
install( FILES kcmkttsd_testmessage.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/ )


