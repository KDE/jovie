
if(KDE4_KTTSD_COMMAND)
	add_subdirectory(command)
endif(KDE4_KTTSD_COMMAND)

if(KDE4_KTTSD_FESTIVAL)
	add_subdirectory(festivalint)
endif(KDE4_KTTSD_FESTIVAL)

if(KDE4_KTTSD_SPEECHD)
	add_subdirectory(speechd)
endif(KDE4_KTTSD_SPEECHD)

