set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_subdirectory( stringreplacer ) 
add_subdirectory( xmltransformer ) 
add_subdirectory( talkerchooser ) 

include_directories( ${CMAKE_SOURCE_DIR}/jovie/libkttsd  )


########### next target ###############

set(testfilter_SRCS main.cpp )


kde4_add_executable(testfilter TEST ${testfilter_SRCS})

target_link_libraries(testfilter  ${KDE4_KDECORE_LIBS} kttsd )


########### install files ###############

install( FILES jovie_filterplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )



