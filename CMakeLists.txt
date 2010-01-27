project(kttsd)

macro_optional_find_package(Speechd)
macro_log_feature(SPEECHD_FOUND "speechd" "Speech Dispatcher provides a high-level device independent layer for speech synthesis" "http://www.freebsoft.org/speechd" FALSE "" "Required to build kttsd, the KDE text-to-speech daemon")

if(Q_WS_X11)
  if(SPEECHD_FOUND)

include(KttsdConfigureChecks.cmake)

configure_file (config-kttsd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kttsd.h )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory( libkttsd ) 
add_subdirectory( filters ) 
add_subdirectory( kttsd ) 
add_subdirectory( kcmkttsmgr ) 
add_subdirectory( kttsmgr ) 
add_subdirectory( icons ) 
  endif(SPEECHD_FOUND)
endif(Q_WS_X11)

