if(Q_WS_X11)

macro_optional_find_package(Speechd)
macro_log_feature(SPEECHD_FOUND "speechd" "Speech Dispatcher provides a high-level device independent layer for speech synthesis" "http://www.freebsoft.org/speechd" FALSE "" "Jovie requires speech dispatcher.")

# if speechd was found use it
if (SPEECHD_FOUND)
  set (SPEECH_LIB ${SPEECHD_LIBRARIES})
endif (SPEECHD_FOUND)

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
else(Q_WS_X11)
	message(FATAL "Cannot build jovie in !X, sorry")
endif(Q_WS_X11)

