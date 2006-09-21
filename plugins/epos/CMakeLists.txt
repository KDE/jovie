
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_eposplugin_PART_SRCS eposconf.cpp eposproc.cpp eposplugin.cpp )

kde4_automoc(${kttsd_eposplugin_PART_SRCS})

kde4_add_ui_files(kttsd_eposplugin_PART_SRCS eposconfwidget.ui )

kde4_add_plugin(kttsd_eposplugin WITH_PREFIX ${kttsd_eposplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_eposplugin )

target_link_libraries(kttsd_eposplugin  ${KDE4_KDECORE_LIBS} kttsd )

install(TARGETS kttsd_eposplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kttsd_eposplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

