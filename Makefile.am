# The build order must be libktts, then libkttsd, then the rest.
SUBDIRS = \
        compat \
        libkttsd \
        players \
        plugins \
        filters \
        kttsd \
        kttsjobmgr \
        kcmkttsmgr \
        kttsmgr \
        app-plugins

messages: rc.cpp
	$(EXTRACTRC) */*.rc */*/*.rc >> rc.cpp
	$(EXTRACTRC) */*.ui */*/*.ui >> rc.cpp
	$(EXTRACTRC) --tag=name --context=FestivalVoiceName plugins/festivalint/voices >> rc.cpp
	$(XGETTEXT) rc.cpp */*.cpp */*.h */*/*.cpp */*/*.h -o $(podir)/kttsd.pot
