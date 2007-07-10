
add_subdirectory( stringreplacer ) 
add_subdirectory( xmltransformer ) 
add_subdirectory( sbd ) 
add_subdirectory( talkerchooser ) 

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

# set(testfilter_SRCS main.cpp )


# kde4_add_test_executable(testfilter ${testfilter_SRCS})

# target_link_libraries(testfilter  ${KDE4_KDECORE_LIBS} kttsd )


########### install files ###############

install( FILES kttsd_filterplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )



