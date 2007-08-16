
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_SRCS 
   main.cpp 
   kspeech.cpp 
   utt.cpp 
   speaker.cpp 
   appdata.cpp 
   configdata.cpp 
   speechdata.cpp 
   threadedplugin.cpp 
   ssmlconvert.cpp 
   filtermgr.cpp 
   talkermgr.cpp 
   speechjob.cpp 
   kspeechadaptor_p.cpp )


kde4_add_executable(kttsd_bin ${kttsd_SRCS})

set_target_properties(kttsd_bin PROPERTIES OUTPUT_NAME kttsd)

target_link_libraries(kttsd_bin
    ${KDE4_KDECORE_LIBS}
    kttsd )

install(TARGETS kttsd_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES SSMLtoPlainText.xsl  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/xslt/ )
install( FILES kttsd.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

