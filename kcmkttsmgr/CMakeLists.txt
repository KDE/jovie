kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_kttsd_PART_SRCS 
   kcmkttsmgr.cpp 
   addtalker.cpp 
   selectevent.cpp 
   kwidgetprobe.cpp )

kde4_automoc(${kcm_kttsd_PART_SRCS})

kde4_add_ui3_files(kcm_kttsd_PART_SRCS selecteventwidget.ui)
kde4_add_ui_files(kcm_kttsd_PART_SRCS kcmkttsmgrwidget.ui addtalkerwidget.ui  )

kde4_add_dcop_skels(kcm_kttsd_PART_SRCS ${CMAKE_INSTALL_PREFIX}/include/kspeechsink.h )

kde4_add_dcop_stubs(kcm_kttsd_PART_SRCS ${CMAKE_INSTALL_PREFIX}/include/kspeech.h )

kde4_add_plugin(kcm_kttsd ${kcm_kttsd_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kttsd )

target_link_libraries(kcm_kttsd  ${KDE4_KIO_LIBS} kttsd  )

install_targets(${PLUGIN_INSTALL_DIR} kcm_kttsd )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kcmkttsd.desktop )
install_files( ${DATA_INSTALL_DIR}/kttsd/ FILES kcmkttsd_testmessage.desktop )

kde4_footer()



#original Makefile.am contents follow:

## Include paths.
#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#	$(CFLAGS_GST) \
#	$(KTTS_KSPEECH_INCLUDE) \
#	$(all_includes)
#
## Let automoc handle all of the metsource files (moc).
#METASOURCES = AUTO
#
##########################################################################
## KCMODULE SECTION
##########################################################################
## This is the kcmodule that gets installed.  It's name is used for all
## of the other Makefile.am variables.
#
#kde_module_LTLIBRARIES = kcm_kttsd.la
#
#kspeech_DIR = $(KTTS_KSPEECH_DIR)
#kspeechsink_DIR = $(KTTS_KSPEECH_DIR)
#
## The source, library search path, and link libraries.
#kcm_kttsd_la_SOURCES = \
#	kspeech.stub \
#	kspeechsink.skel \
#	kcmkttsmgr.cpp \
#	kcmkttsmgrwidget.ui \
#	addtalker.cpp \
#	addtalkerwidget.ui \
#	selectevent.cpp \
#	selecteventwidget.ui \
#	kwidgetprobe.cpp
#
#kcm_kttsd_la_LDFLAGS = -avoid-version -module -no-undefined $(all_libraries)
#kcm_kttsd_la_LIBADD = \
#	$(top_builddir)/kttsd/libkttsd/libkttsd.la \
#	$(LIB_KIO) 
#
## Header files that should not be installed.
#noinst_HEADERS = \
#	kcmkttsmgr.h \
#	kcmkttsmgrwidget.h \
#	kcmkttsmgrwidget.ui.h \
#	addtalker.h \
#	selectevent.h
#
## K Menu entry.
#xdg_apps_DATA = kcmkttsd.desktop
#
## Install data files.
#kcmkttsddatadir = $(kde_datadir)/kttsd/
#kcmkttsddata_DATA = kcmkttsd_testmessage.desktop
