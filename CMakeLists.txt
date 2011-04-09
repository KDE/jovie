project(kttsd)

if (Q_WS_X11)
  macro_optional_find_package(Speechd)
  macro_log_feature(SPEECHD_FOUND "speechd" "Speech Dispatcher provides a high-level device independent layer for speech synthesis" "http://www.freebsoft.org/speechd" TRUE "" "Jovie requires speech dispatcher.")

  if (SPEECHD_FOUND)
    include(KttsdConfigureChecks.cmake)

    configure_file (config-jovie.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-jovie.h )

    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    add_subdirectory( libkttsd )
    add_subdirectory( filters )
    add_subdirectory( jovie )
    add_subdirectory( kcmkttsmgr )
    add_subdirectory( icons )
  endif (SPEECHD_FOUND)
else (Q_WS_X11)
  message(FATAL_ERROR "Jovie requires a X11-based system.")
endif (Q_WS_X11)

