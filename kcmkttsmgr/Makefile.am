# Include paths.  INCLUDES is maintained by KDevelop, AM_CPPFLAGS is the preferred variable,
# so keep them synchronized.
INCLUDES = \
	-I$(top_srcdir)/kttsd/libktts \
	-I$(top_srcdir)/kttsd/libkttsd \
	$(CFLAGS_GST) \
	$(all_includes)
AM_CPPFLAGS = \
	-I$(top_srcdir)/kttsd/libktts \
	-I$(top_srcdir)/kttsd/libkttsd \
	$(CFLAGS_GST) \
	$(all_includes)

# Let automoc handle all of the metsource files (moc).
METASOURCES = AUTO

#########################################################################
# KCMODULE SECTION
#########################################################################
# This is the kcmodule that gets installed.  It's name is used for all
# of the other Makefile.am variables.

# Build two identical libraries, but with different names.
# Once all apps have stopped using kcm_kttsmgr, remove it.
kde_module_LTLIBRARIES = kcm_kttsd.la kcm_kttsmgr.la

# The source, library search path, and link libraries.
kcm_kttsd_la_SOURCES = \
	kcmkttsmgr.cpp \
	kcmkttsmgrwidget.ui \
	addtalker.cpp \
	addtalkerwidget.ui
kcm_kttsd_la_LDFLAGS = -avoid-version -module -no-undefined 
kcm_kttsd_la_LIBADD = $(top_builddir)/kttsd/libktts/libktts.la $(top_builddir)/kttsd/libkttsd/libkttsd.la $(LIB_QT) $(LIB_KIO) $(LIB_KDECORE)

kcm_kttsmgr_la_SOURCES = \
	kcmkttsmgr.cpp \
	kcmkttsmgrwidget.ui \
	addtalker.cpp \
	addtalkerwidget.ui
kcm_kttsmgr_la_LDFLAGS = -avoid-version -module -no-undefined 
kcm_kttsmgr_la_LIBADD = $(top_builddir)/kttsd/libktts/libktts.la $(top_builddir)/kttsd/libkttsd/libkttsd.la $(LIB_QT) $(LIB_KIO) $(LIB_KDECORE)

# Header files that should not be installed.
noinst_HEADERS = kcmkttsmgr.h kcmkttsmgrwidget.h kcmkttsmgrwidget.ui.h \
	addtalker.h

# Install desktop file to application directory.
apps_DATA = kcmkttsd.desktop kcmkttsmgr.desktop
appsdir = $(kde_appsdir)/Settings/Accessibility

# K Menu entry.
xdg_apps_DATA = kcmkttsd.desktop

# Translation message files.
messages: rc.cpp
	$(XGETTEXT) -nki18n *.cpp -o $(podir)/kcmkttsmgr.pot

