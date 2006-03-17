kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kttsd/libkttsd ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kttsd_xmltransformerplugin_PART_SRCS xmltransformerconf.cpp xmltransformerproc.cpp xmltransformerplugin.cpp )

kde4_automoc(${kttsd_xmltransformerplugin_PART_SRCS})

kde4_add_ui3_files(kttsd_xmltransformerplugin_PART_SRCS xmltransformerconfwidget.ui )

kde4_add_plugin(kttsd_xmltransformerplugin WITH_PREFIX ${kttsd_xmltransformerplugin_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kttsd_xmltransformerplugin )

target_link_libraries(kttsd_xmltransformerplugin  ${QT_AND_KDECORE_LIBS} kttsd )

install_targets(${LIB_INSTALL_DIR}/kde4 kttsd_xmltransformerplugin )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kttsd/xmltransformer/ FILES xhtml2ssml.xsl xhtml2ssml_simple.xsl )
install_files( ${SERVICES_INSTALL_DIR} FILES kttsd_xmltransformerplugin.desktop )

kde4_footer()



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
