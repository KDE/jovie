project(kttsd)

include(KttsdConfigureChecks.cmake)

configure_file (config-kttsd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kttsd.h )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory( libkttsd ) 
add_subdirectory( filters ) 
add_subdirectory( kttsd ) 
add_subdirectory( kcmkttsmgr ) 
add_subdirectory( kttsmgr ) 
add_subdirectory( icons ) 

