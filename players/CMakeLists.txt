
if(KDE4_KTTSD_PHONON)
        add_subdirectory(phononplayer)
endif(KDE4_KTTSD_PHONON)

if(ASOUND_LIBRARY AND KDE4_KTTSD_ALSA)
	add_subdirectory(alsaplayer)
endif(ASOUND_LIBRARY AND KDE4_KTTSD_ALSA)



########### install files ###############

install( FILES kttsd_audioplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
