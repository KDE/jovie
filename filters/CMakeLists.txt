
add_subdirectory( stringreplacer ) 
add_subdirectory( xmltransformer ) 
add_subdirectory( sbd ) 
add_subdirectory( talkerchooser ) 

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(testfilter_SRCS main.cpp )

kde4_automoc(${testfilter_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(testfilter ${testfilter_SRCS})

target_link_libraries(testfilter  ${KDE4_KDECORE_LIBS} kttsd )

endif(KDE4_BUILD_TESTS)

########### install files ###############

install( FILES kttsd_filterplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )




#original Makefile.am contents follow:

## Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
## so keep them synchronized.
#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd \
#	-I$(kde_includes)/arts \
#	$(CFLAGS_GST) \
#	$(all_includes)
#
#SUBDIRS = stringreplacer xmltransformer sbd talkerchooser
#
##########################################################################
## APPLICATION SECTION
##########################################################################
## This is the program that gets installed.  It's name is used for all
## of the other Makefile.am variables.
#check_PROGRAMS = testfilter
#
#testfilter_SOURCES = main.cpp
#
#testfilter_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testfilter_LDADD = \
#	$(top_builddir)/kttsd/libkttsd/libkttsd.la \
#	$(LIB_KDECORE)
#
#kde_servicetypes_DATA = kttsd_filterplugin.desktop
#
