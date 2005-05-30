#kdevelop: gstplayer_DIR = gstplayer
if include_kttsd_gstplayer
    gstplayer_DIR = gstplayer
endif

#kdevelop: alsaplayer_DIR = alsaplayer
if include_kttsd_alsaplayer
    alsaplayer_DIR = alsaplayer
endif

SUBDIRS = \
    artsplayer \
    $(alsaplayer_DIR) \
    $(gstplayer_DIR)

kde_servicetypes_DATA = kttsd_audioplugin.desktop
