
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_stringreplacerplugin_PART_SRCS stringreplacerconf.cpp stringreplacerproc.cpp stringreplacerplugin.cpp )

kde4_automoc(${kttsd_stringreplacerplugin_PART_SRCS})

kde4_add_ui_files(kttsd_stringreplacerplugin_PART_SRCS stringreplacerconfwidget.ui editreplacementwidget.ui )

kde4_add_plugin(kttsd_stringreplacerplugin WITH_PREFIX ${kttsd_stringreplacerplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_stringreplacerplugin )

target_link_libraries(kttsd_stringreplacerplugin  ${KDE4_KDECORE_LIBS} kttsd )

install(TARGETS kttsd_stringreplacerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kttsd/stringreplacer/ FILES 
    abbreviations.xml
    chat.xml
    chat-de.xml
    emoticons.xml
    polish_festival_fixes.xml
    polish_festival_unspeakables.xml
    kmail.xml
    festival_unspeakable_chars.xml
    qt2plaintext.xml )
    
install( FILES kttsd_stringreplacerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#	$(all_includes)
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkttsd_stringreplacerplugin.la
#
#libkttsd_stringreplacerplugin_la_SOURCES = \
#	stringreplacerconfwidget.ui \
#	editreplacementwidget.ui \
#	stringreplacerconf.cpp \
#	stringreplacerproc.cpp \
#	stringreplacerplugin.cpp
#libkttsd_stringreplacerplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
#libkttsd_stringreplacerplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la
#
#services_DATA = kttsd_stringreplacerplugin.desktop
#servicesdir = $(kde_servicesdir)
#
## Install data files.
#stringreplacerdatadir = $(kde_datadir)/kttsd/stringreplacer/
#stringreplacerdata_DATA = \
#	abbreviations.xml \
#	chat.xml \
#	emoticons.xml \
#	polish_festival_fixes.xml \
#	polish_festival_unspeakables.xml \
#	kmail.xml \
#	festival_unspeakable_chars.xml \
#	qt2plaintext.xml
#
#noinst_HEADERS =  stringreplacerconfwidget.h editreplacementwidget.h
