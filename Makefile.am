# The build order must be libktts, then libkttsd, then the rest.
SUBDIRS = \
        libkttsd \
        players \
        plugins \
        filters \
        kttsd \
        icons \
        kttsjobmgr \
        kcmkttsmgr \
        kttsmgr \
	app-plugins

# Generate API documentation.
DOXYGEN_EXCLUDE = \
        players \
        plugins \
        filters \
        kttsd \
        icons \
        kttsjobmgr \
        kcmkttsmgr \
        kttsmgr \
        kcmkttsd \
	app-plugins

include $(top_srcdir)/admin/Doxyfile.am

