kde4_header()

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

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##kdevelop: festivalint_DIR = festivalint
#if include_kttsd_festivalint
#  festivalint_DIR = festivalint
#endif
#
##kdevelop: freetts_DIR = freetts
#if include_kttsd_freetts
#  freetts_DIR = freetts
#endif
#
##kdevelop: command_DIR = command
#if include_kttsd_command
#  command_DIR = command
#endif
#
##kdevelop: hadifix_DIR = hadifix
#if include_kttsd_hadifix
#  hadifix_DIR = hadifix
#endif
#
##kdevelop: flite_DIR = flite
#if include_kttsd_flite
#  flite_DIR = flite
#endif
#
##kdevelop: epos_DIR = epos
#if include_kttsd_epos
#  epos_DIR = epos
#endif
#
#SUBDIRS = \
#        $(festivalint_DIR) \
#        $(freetts_DIR) \
#        $(command_DIR) \
#        $(hadifix_DIR) \
#        $(flite_DIR) \
#        $(epos_DIR)
