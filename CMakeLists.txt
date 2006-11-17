project(kttsd)

configure_file (config-kttsd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kttsd.h )

add_subdirectory( libkttsd ) 
add_subdirectory( players ) 
add_subdirectory( plugins ) 
add_subdirectory( filters ) 
add_subdirectory( kttsd ) 
add_subdirectory( kttsjobmgr ) 
add_subdirectory( kcmkttsmgr ) 
add_subdirectory( kttsmgr ) 
add_subdirectory( app-plugins ) 
add_subdirectory( kspeak )
add_subdirectory( icons ) 

