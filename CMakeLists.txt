project(kttsd)

include(KttsdConfigureChecks.cmake)
include(FindAlsa)

configure_file (config-kttsd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kttsd.h )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory( libkttsd ) 
add_subdirectory( players ) 
add_subdirectory( plugins ) 
add_subdirectory( filters ) 
add_subdirectory( kttsd ) 
add_subdirectory( kttsjobmgr ) 
add_subdirectory( kcmkttsmgr ) 
add_subdirectory( kttsmgr ) 
add_subdirectory( kspeak )
add_subdirectory( icons ) 

