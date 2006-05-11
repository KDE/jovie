kde4_header()
project(kttsd)

add_subdirectory( compat ) 
add_subdirectory( libkttsd ) 
add_subdirectory( players ) 
add_subdirectory( plugins ) 
add_subdirectory( filters ) 
add_subdirectory( kttsd ) 
add_subdirectory( kttsjobmgr ) 
add_subdirectory( kcmkttsmgr ) 
add_subdirectory( kttsmgr ) 
add_subdirectory( app-plugins ) 
add_subdirectory( icons ) 



########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

## The build order must be libktts, then libkttsd, then the rest.
#SUBDIRS = \
#        compat \
#        libkttsd \
#        players \
#        plugins \
#        filters \
#        kttsd \
#        kttsjobmgr \
#        kcmkttsmgr \
#        kttsmgr \
#        app-plugins \
#	icons
#
#messages: rc.cpp
#	$(EXTRACTRC) */*.rc */*/*.rc >> rc.cpp
#	$(EXTRACTRC) */*.ui */*/*.ui >> rc.cpp
#	$(EXTRACTRC) --tag=name --context=FestivalVoiceName plugins/festivalint/voices >> rc.cpp
#	$(XGETTEXT) rc.cpp */*.cpp */*.h */*/*.cpp */*/*.h -o $(podir)/kttsd.pot
