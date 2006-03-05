kde4_header()

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(festivalint_DIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(freetts_DIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(command_DIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(hadifix_DIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(flite_DIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(epos_DIR)")
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
