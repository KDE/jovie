



########### next target ###############

set(kttsmgr_SRCS 
    kttsmgr.cpp )

qt4_add_dbus_interfaces(kttsmgr_SRCS org.kde.KSpeech.xml )

kde4_automoc(${kttsmgr_SRCS})

kde4_add_executable(kttsmgr ${kttsmgr_SRCS})

target_link_libraries(kttsmgr
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS} )

install_targets(/bin kttsmgr )


########### install files ###############

install( FILES kttsmgr.desktop  DESTINATION  ${XDG_APPS_DIR} )




#original Makefile.am contents follow:

## Include paths.
#INCLUDES = \
#	$(KTTS_KSPEECH_INCLUDE) \
#	$(all_includes)
#
## Let automoc handle all of the metsource files (moc).
#METASOURCES = AUTO
#
##########################################################################
## APPLICATION SECTION
##########################################################################
## This is the program that gets installed.  It's name is used for all
## of the other Makefile.am variables.
#bin_PROGRAMS = kttsmgr
#
#kspeech_DIR = $(KTTS_KSPEECH_DIR)
#kspeechsink_DIR = $(KTTS_KSPEECH_DIR)
#
## The source, library search path, and link libraries.
#kttsmgr_SOURCES =  \
#	kspeech.stub \
#	kspeechsink.skel \
#	kttsmgr.cpp
#
#kttsmgr_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kttsmgr_LDADD = \
#	$(LIB_KUTILS) \
#	$(LIB_KDEUI)
#
## Header files that should not be installed.
#noinst_HEADERS = kttsmgr.h
#
## K Menu entry.
#xdg_apps_DATA = kttsmgr.desktop
