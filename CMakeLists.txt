if(Q_WS_X11)

macro_optional_find_package(Speechd)
macro_log_feature(SPEECHD_FOUND "speechd" "Speech Dispatcher provides a high-level device independent layer for speech synthesis" "http://www.freebsoft.org/speechd" FALSE "" "Jovie requires either speech dispatcher or opentts.")

macro_optional_find_package(Opentts)
macro_log_feature(OPENTTS_FOUND "opentts" "Open Text To Speech provides a high-level device independent layer for speech synthesis" "http://www.opentts.org" FALSE "" "Jovie requires either speech dispatcher or opentts")

# if speechd was found use it
if (SPEECHD_FOUND)
  set (SPEECH_LIB speechd)
endif (SPEECHD_FOUND)

# prefer opentts if it was also found (or if it was the only one found)
if (OPENTTS_FOUND)
  set (SPEECH_LIB opentts)
endif (OPENTTS_FOUND)

# we found speechd or opentts, so continue
  if(DEFINED SPEECH_LIB)

project(kttsd)

include(KttsdConfigureChecks.cmake)

configure_file (config-jovie.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-jovie.h )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory( libkttsd ) 
add_subdirectory( filters ) 
add_subdirectory( jovie ) 
add_subdirectory( kcmkttsmgr ) 
add_subdirectory( icons ) 
  endif(DEFINED SPEECH_LIB)
endif(Q_WS_X11)

