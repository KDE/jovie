kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############
qt4_generate_moc(player.h player.moc.cpp)

set(kttsd_LIB_SRCS player.moc.cpp 
   pluginproc.cpp 
   pluginconf.cpp 
   testplayer.cpp 
   stretcher.cpp 
   talkercode.cpp 
   filterproc.cpp 
   filterconf.cpp 
   utils.cpp 
   talkerlistmodel.cpp 
   selecttalkerdlg.cpp 
   notify.cpp )

kde4_automoc(${kttsd_LIB_SRCS})

kde4_add_ui_files(kttsd_LIB_SRCS selecttalkerwidget.ui )

kde4_add_library(kttsd SHARED ${kttsd_LIB_SRCS})

target_link_libraries(kttsd  ${KDE4_KDE3SUPPORT_LIBRARIES} )

set_target_properties(kttsd PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kttsd )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES kttsd_synthplugin.desktop )

kde4_footer()



#original Makefile.am contents follow:

## Include paths.
#INCLUDES = $(all_includes)
#
## Let automoc handle all of the metsource files (moc).
#METASOURCES = AUTO
#
##########################################################################
## LIBRARY SECTION
##########################################################################
## This is the library that gets installed.  It's name is used for all
## of the other Makefile.am variables.
#lib_LTLIBRARIES = libkttsd.la
#
## The source, library search path, and link libraries.
#libkttsd_la_SOURCES = \
#	pluginproc.cpp \
#	pluginconf.cpp \
#	testplayer.cpp \
#	stretcher.cpp \
#	talkercode.cpp \
#	filterproc.cpp \
#	filterconf.cpp \
#	utils.cpp \
#	talkerlistmodel.cpp \
#	selecttalkerdlg.cpp \
#	selecttalkerwidget.ui \
#	notify.cpp
#
#libkttsd_la_LDFLAGS = -version-info 1:0:0 $(all_libraries) -no-undefined
#libkttsd_la_LIBADD = $(LIB_KIO)
#
## Header files that should not be installed.
#noinst_HEADERS = \
#	pluginproc.h \
#	pluginconf.h \
#	player.h \
#	testplayer.h \
#	stretcher.h \
#	talkercode.h \
#	talkerlistmodel.h \
#	utils.h \
#	kdeexportfix.h \
#	selecttalkerdlg.h
#
## This library is installed as a service.
#servicetypes_DATA = kttsd_synthplugin.desktop
#servicetypesdir = $(kde_servicetypesdir)
