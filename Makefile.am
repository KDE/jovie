# The build order must be libktts, then libkttsd, then the rest.
SUBDIRS = \
        libktts \
        libkttsd \
        players \
        plugins \
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
        kttsd \
        icons \
        kttsjobmgr \
        kcmkttsmgr \
        kttsmgr \
        kcmkttsd \
        app-plugins

include $(top_srcdir)/admin/Doxyfile.am

