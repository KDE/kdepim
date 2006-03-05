kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( /include/ktnef FILES ktnefparser.h ktnefattach.h ktnefproperty.h  	ktnefpropertyset.h ktnefmessage.h ktnefwriter.h ktnefdefs.h )

kde4_footer()



#original Makefile.am contents follow:

#ktnefinclude_HEADERS = ktnefparser.h ktnefattach.h ktnefproperty.h \
#	ktnefpropertyset.h ktnefmessage.h ktnefwriter.h ktnefdefs.h
#ktnefincludedir = $(includedir)/ktnef
#INCLUDES = $(all_includes)
