
if(KDE4_KTTSD_PHONON)
        add_subdirectory(phononplayer)
endif(KDE4_KTTSD_PHONON)

if(KDE4_KTTSD_ALSA)
	add_subdirectory(alsaplayer)
endif(KDE4_KTTSD_ALSA)



########### install files ###############

install( FILES kttsd_audioplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )




#original Makefile.am contents follow:

##kdevelop: artsplayer_DIR = artsplayer
#if include_kttsd_artsplayer
#    artsplayer_DIR = artsplayer
#endif
#
##kdevelop: gstplayer_DIR = gstplayer
#if include_kttsd_gstplayer
#    gstplayer_DIR = gstplayer
#endif
#
##kdevelop: alsaplayer_DIR = alsaplayer
#if include_kttsd_alsaplayer
#    alsaplayer_DIR = alsaplayer
#endif
#
##kdevelop: akodeplayer_DIR = akodeplayer
#if include_kttsd_akodeplayer
#    akodeplayer_DIR = akodeplayer
#endif
#
#SUBDIRS = \
#    $(artsplayer_DIR) \
#    $(alsaplayer_DIR) \
#    $(gstplayer_DIR) \
#    $(akodeplayer_DIR)
#
#kde_servicetypes_DATA = kttsd_audioplugin.desktop
