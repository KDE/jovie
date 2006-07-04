



########### install files ###############

install( FILES dcoptexttospeech.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install_files( /include FILES kspeech.h kspeechsink.h )




#original Makefile.am contents follow:

## This is necessary for "make bcheck" to work.
#INCLUDES = $(all_includes)
#
## Header files to be installed.
#include_HEADERS = kspeech.h kspeechsink.h
#
## Define DCOP/Text-to-Speech Service Type.
#kde_servicetypes_DATA = dcoptexttospeech.desktop
