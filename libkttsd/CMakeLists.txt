
set(kttsd_LIB_SRCS
   pluginproc.cpp 
   pluginconf.cpp 
   testplayer.cpp 
   stretcher.cpp 
   talkercode.cpp 
   filterproc.cpp 
   filterconf.cpp 
   utils.cpp 
   talkerlistmodel.cpp 
   selecttalkerdlg.cpp 
   selectlanguagedlg.cpp 
   notify.cpp )

kde4_add_ui_files(kttsd_LIB_SRCS selecttalkerwidget.ui )

kde4_automoc(${kttsd_LIB_SRCS})

kde4_add_library(kttsd SHARED ${kttsd_LIB_SRCS})

target_link_libraries(kttsd
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS} )

set_target_properties(kttsd PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kttsd  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES kttsd_synthplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

