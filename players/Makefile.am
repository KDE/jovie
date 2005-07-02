#kdevelop: artsplayer_DIR = artsplayer
if include_kttsd_artsplayer
    artsplayer_DIR = artsplayer
endif

#kdevelop: gstplayer_DIR = gstplayer
if include_kttsd_gstplayer
    gstplayer_DIR = gstplayer
endif

#kdevelop: alsaplayer_DIR = alsaplayer
if include_kttsd_alsaplayer
    alsaplayer_DIR = alsaplayer
endif

#kdevelop: akodeplayer_DIR = akodeplayer
if include_kttsd_akodeplayer
    akodeplayer_DIR = akodeplayer
endif

SUBDIRS = \
    $(artsplayer_DIR) \
    $(alsaplayer_DIR) \
    $(gstplayer_DIR) \
    $(akodeplayer_DIR)

kde_servicetypes_DATA = kttsd_audioplugin.desktop
