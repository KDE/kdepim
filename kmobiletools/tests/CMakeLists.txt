
add_subdirectory( kserialdeviceemulator )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = kserialdeviceemulator
#
#KDE_CXXFLAGS = -UQT_NO_ASCII_CAST
#
