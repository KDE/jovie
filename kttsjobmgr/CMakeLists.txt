
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${CMAKE_BINARY_DIR}/kttsd/libkttsd )


########### next target ###############

set(kttsjobmgrpart_PART_SRCS
    jobinfolistmodel.cpp
    kttsjobmgr.cpp )

kde4_automoc(${kttsjobmgrpart_PART_SRCS})

qdbus_add_interfaces(kttsjobmgrpart_PART_SRCS org.kde.KSpeech.xml )

kde4_add_plugin(kttsjobmgrpart WITH_PREFIX ${kttsjobmgrpart_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsjobmgrpart )

target_link_libraries(kttsjobmgrpart  ${KDE4_KDECORE_LIBS} kttsd kparts )

install_targets(${PLUGIN_INSTALL_DIR} kttsjobmgrpart )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kttsjobmgr.desktop )




#original Makefile.am contents follow:

## Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
## so keep them synchronized.
#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#	$(KTTS_KSPEECH_INCLUDE) \
#	$(all_includes)
#
## Let automoc handle all of the metsource files (moc).
#METASOURCES = AUTO
#
##########################################################################
## KPART SECTION
##########################################################################
## This is the kpart that gets installed.  It's name is used for all
## of the other Makefile.am variables.
#kde_module_LTLIBRARIES =  libkttsjobmgrpart.la
#
#kspeech_DIR = $(KTTS_KSPEECH_DIR)
#kspeechsink_DIR = $(KTTS_KSPEECH_DIR)
#
## The source, library search path, and link libraries.
#libkttsjobmgrpart_la_SOURCES = \
#	kspeech.stub kspeechsink.skel \
#	kttsjobmgr.cpp
#
#libkttsjobmgrpart_la_LDFLAGS = -avoid-version -module -no-undefined $(all_libraries) $(KDE_PLUGIN)
#libkttsjobmgrpart_la_LIBADD = \
#	$(top_builddir)/kttsd/libkttsd/libkttsd.la \
#	$(LIB_KFILE) $(LIBVM) $(LIB_KPARTS)
#
## Header files not to be installed.
#noinst_HEADERS = 
#
## Install desktop file to standard services directory.
#kde_services_DATA =  kttsjobmgr.desktop
#
## This rc file should never have been installed.
## createGUI not supported within a KCModule.
#install-data-local:
#	rm -f $(DESTDIR)$(kde_datadir)/kttsjobmgr/kttsjobmgrui.rc
