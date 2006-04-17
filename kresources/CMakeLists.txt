kde4_header()

add_subdirectory( lib ) 
add_subdirectory( remote ) 
add_subdirectory( egroupware ) 
add_subdirectory( kolab ) 
add_subdirectory( slox ) 
add_subdirectory( featureplan ) 
add_subdirectory( groupdav ) 
add_subdirectory( birthdays ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = lib remote egroupware kolab slox featureplan groupdav birthdays
