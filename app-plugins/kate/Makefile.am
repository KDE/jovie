# Allow build from within kdebase or kdeaccessibility.
INCLUDES    = \
	-I$(top_srcdir)/kate/interfaces \
	-I$(kde_includes)/kate \
	$(all_includes)

METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = ktexteditor_kttsd.la

ktexteditor_kttsd_la_SOURCES = katekttsd.cpp
ktexteditor_kttsd_la_LIBADD  = -lktexteditor
ktexteditor_kttsd_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

kttsddatadir = $(kde_datadir)/ktexteditor_kttsd
kttsddata_DATA = ktexteditor_kttsdui.rc

kde_services_DATA = ktexteditor_kttsd.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/ktexteditor_kttsd.pot
