
if(KDE4_KTTSD_COMMAND)
	add_subdirectory(command)
endif(KDE4_KTTSD_COMMAND)

if(KDE4_KTTSD_EPOS)
	add_subdirectory(epos)
endif(KDE4_KTTSD_EPOS)

if(KDE4_KTTSD_FESTIVAL)
	add_subdirectory(festivalint)
endif(KDE4_KTTSD_FESTIVAL)

if(KDE4_KTTSD_FLITE)
	add_subdirectory(flite)
endif(KDE4_KTTSD_FLITE)

if(KDE4_KTTSD_FREETTS)
	add_subdirectory(freetts)
endif(KDE4_KTTSD_FREETTS)

if(KDE4_KTTSD_HADIFIX)
	add_subdirectory(hadifix)
endif(KDE4_KTTSD_HADIFIX)

