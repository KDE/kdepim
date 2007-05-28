
add_subdirectory( libkmobiletoolsengineui )
add_subdirectory( at_engine )

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(DIRGAMMU)")


#original Makefile.am contents follow:

#KDE_OPTIONS = nofinal
##if have_gammu
##    DIRGAMMU = gammu_engine
##endif
#
#METASOURCES = AUTO
#
#SUBDIRS = at_engine $(DIRGAMMU) 
#
#
#
#
#KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
#
