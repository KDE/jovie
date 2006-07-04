
include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd  )


########### next target ###############

set(kttsd_xmltransformerplugin_PART_SRCS xmltransformerconf.cpp xmltransformerproc.cpp xmltransformerplugin.cpp )

kde4_automoc(${kttsd_xmltransformerplugin_PART_SRCS})

kde4_add_ui_files(kttsd_xmltransformerplugin_PART_SRCS xmltransformerconfwidget.ui )

kde4_add_plugin(kttsd_xmltransformerplugin WITH_PREFIX ${kttsd_xmltransformerplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_xmltransformerplugin )

target_link_libraries(kttsd_xmltransformerplugin  ${KDE4_KDECORE_LIBS} kttsd )

install(TARGETS kttsd_xmltransformerplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES xhtml2ssml.xsl xhtml2ssml_simple.xsl  DESTINATION  ${DATA_INSTALL_DIR}/kttsd/xmltransformer/ )
install( FILES kttsd_xmltransformerplugin.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

#INCLUDES = \
#	-I$(top_srcdir)/kttsd/libkttsd -I$(top_builddir)/kttsd/libkttsd \
#	$(all_includes)
#
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkttsd_xmltransformerplugin.la
#
#libkttsd_xmltransformerplugin_la_SOURCES = \
#	xmltransformerconfwidget.ui \
#	xmltransformerconf.cpp \
#	xmltransformerproc.cpp \
#	xmltransformerplugin.cpp
#libkttsd_xmltransformerplugin_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
#libkttsd_xmltransformerplugin_la_LIBADD = $(top_builddir)/kttsd/libkttsd/libkttsd.la
#
#services_DATA = kttsd_xmltransformerplugin.desktop
#servicesdir = $(kde_servicesdir)
#
## Install data files.
#xmltransformerdatadir = $(kde_datadir)/kttsd/xmltransformer/
#xmltransformerdata_DATA = xhtml2ssml.xsl xhtml2ssml_simple.xsl
#
#noinst_HEADERS =  xmltransformerconfwidget.h
