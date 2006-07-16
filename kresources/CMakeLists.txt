
message( STATUS "Kolab and GroupWise resources are disabled." )
add_subdirectory( lib ) 
add_subdirectory( remote ) 
add_subdirectory( egroupware ) 
#add_subdirectory( kolab ) 
add_subdirectory( slox ) 
add_subdirectory( featureplan ) 
add_subdirectory( groupdav ) 
add_subdirectory( birthdays ) 




########### install files ###############





#original Makefile.am contents follow:

#SUBDIRS = lib remote egroupware kolab slox featureplan groupdav birthdays
