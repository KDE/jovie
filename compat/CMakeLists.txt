kde4_header()

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KTTS_ICONS_DIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KTTS_INTERFACES_DIR)")



########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = \
#	$(KTTS_ICONS_DIR) \
#	$(KTTS_INTERFACES_DIR)
