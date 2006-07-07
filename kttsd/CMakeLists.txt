
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

kde4_automoc(${kttsd_SRCS})

kde4_add_executable(kttsd_bin ${kttsd_SRCS})

set_target_properties(kttsd_bin PROPERTIES OUTPUT_NAME kttsd)

target_link_libraries(kttsd_bin
    ${KDE4_KDECORE_LIBS}
    kttsd )

install(TARGETS kttsd_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES SSMLtoPlainText.xsl  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/xslt/ )
install( FILES kttsd.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

## Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
## so keep them synchronized.
#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd \
#	-I$(kde_includes)/arts \
#	$(KTTS_KSPEECH_INCLUDE) \
#	$(all_includes)
#
## Let am_edit/unsermake handle all of the metasource files (moc).
#METASOURCES = AUTO
#
##########################################################################
## APPLICATION SECTION
##########################################################################
## This is the program that gets installed.  It's name is used for all
## of the other Makefile.am variables.
#bin_PROGRAMS = kttsd
#
#kspeech_DIR = $(KTTS_KSPEECH_DIR)
#kspeechsink_DIR = $(KTTS_KSPEECH_DIR)
#
## The source, library search path, and link libraries.
## Note: .skel files cause DCOPIDL compiler to generate _skel.cpp file and compile it.
#kttsd_SOURCES = \
#    kspeech.skel kspeechsink.stub\
#	main.cpp \
#	kttsd.cpp \
#	speaker.cpp \
#	speechdata.cpp \
#	kttsd.skel \
#	threadedplugin.cpp \
#	ssmlconvert.cpp \
#	filtermgr.cpp \
#	talkermgr.cpp
#
#kttsd_LDFLAGS = -avoid-version -module $(all_libraries) $(KDE_RPATH)
#kttsd_LDADD = \
#	$(top_builddir)/kttsd/libkttsd/libkttsd.la \
#	$(LIB_KDECORE) \
#	$(LIB_KIO) \
#	$(LIB_KUTILS)
#
## Install desktop file to standard services directory.
#kde_services_DATA = kttsd.desktop
#
## Install data.
#kttsddatadir = $(kde_datadir)/kttsd/xslt/
#kttsddata_DATA = SSMLtoPlainText.xsl
#
#noinst_HEADERS = threadedplugin.h ssmlconvert.h
#
