
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${CMAKE_BINARY_DIR}/kttsd/libkttsd )


########### next target ###############

set(kttsjobmgrpart_PART_SRCS
    jobinfolistmodel.cpp
    kttsjobmgr.cpp )

kde4_automoc(${kttsjobmgrpart_PART_SRCS})

qt4_add_dbus_interfaces(kttsjobmgrpart_PART_SRCS ${DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml )

kde4_add_plugin(kttsjobmgrpart WITH_PREFIX ${kttsjobmgrpart_PART_SRCS})



target_link_libraries(kttsjobmgrpart  ${KDE4_KPARTS_LIBS} kttsd  )

install(TARGETS kttsjobmgrpart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kttsjobmgr.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

