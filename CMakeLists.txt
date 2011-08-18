project(kttsd)

set(CMAKE_MODULE_PATH "${kttsd_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

if (Q_WS_X11)
  macro_optional_find_package(Speechd)
  macro_log_feature(SPEECHD_FOUND "speechd" "Speech Dispatcher provides a high-level device independent layer for speech synthesis" "http://www.freebsoft.org/speechd" FALSE "" "Jovie requires speech dispatcher.")

  if (SPEECHD_FOUND)
    configure_file (config-jovie.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-jovie.h )

    include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libkttsd ${CMAKE_CURRENT_SOURCE_DIR}/libkttsd)

    add_subdirectory( doc )
    add_subdirectory( libkttsd )
    add_subdirectory( filters )
    add_subdirectory( jovie )
    add_subdirectory( kcmkttsmgr )
    add_subdirectory( icons )
  endif (SPEECHD_FOUND)
else (Q_WS_X11)
  message(FATAL_ERROR "Jovie requires a X11-based system.")
endif (Q_WS_X11)

macro_display_feature_log()

