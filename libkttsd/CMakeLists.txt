
include_directories( ${SPEECHD_INCLUDE_DIR} )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=2405)

set(kttsd_LIB_SRCS
   talkercode.cpp 
   filterproc.cpp 
   filterconf.cpp 
   talkerlistmodel.cpp ) 

kde4_add_library(kttsd SHARED ${kttsd_LIB_SRCS})

target_link_libraries(kttsd
    ${SPEECHD_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${QT_QTXML_LIBRARY}
    )

set_target_properties(kttsd PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kttsd  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

#install( FILES kttsd_synthplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

