# The build order must be libktts, then libkttsd, then the rest.
SUBDIRS = \
        libktts \
        libkttsd \
        plugins \
        kttsd \
        icons \
        kttsjobmgr \
        kcmkttsmgr \
        kttsmgr \
        kcmkttsd \
        app-plugins

# Generate API documentation.
DOXYGEN_EXCLUDE = \
        plugins \
        kttsd \
        icons \
        kttsjobmgr \
        kcmkttsmgr \
        kttsmgr \
        kcmkttsd \
        app-plugins

include $(top_srcdir)/admin/Doxyfile.am

