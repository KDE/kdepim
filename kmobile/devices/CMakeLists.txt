kde4_header()

add_subdirectory( pics ) 
add_subdirectory( skeleton ) 
add_subdirectory( digicam ) 
add_subdirectory( gammu ) 

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KMOBILE_GNOKIIDIR)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#if include_gnokii_device
#KMOBILE_GNOKIIDIR = gnokii
#endif
#
#SUBDIRS = pics skeleton $(KMOBILE_GNOKIIDIR) digicam gammu 
