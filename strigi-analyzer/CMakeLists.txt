kde4_header()

add_subdirectory( vcf ) 
add_subdirectory( ics ) 

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(PILOTPLUGIN)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#if compile_kpilot
#PILOTPLUGIN = palm-databases
#endif
#
#SUBDIRS=vcf ics $(PILOTPLUGIN)
#
