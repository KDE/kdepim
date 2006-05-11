kde4_header()




########### install files ###############

install_files( /include/ktnef FILES ktnefparser.h ktnefattach.h ktnefproperty.h  	ktnefpropertyset.h ktnefmessage.h ktnefwriter.h ktnefdefs.h )

kde4_footer()



#original Makefile.am contents follow:

#ktnefinclude_HEADERS = ktnefparser.h ktnefattach.h ktnefproperty.h \
#	ktnefpropertyset.h ktnefmessage.h ktnefwriter.h ktnefdefs.h
#ktnefincludedir = $(includedir)/ktnef
#INCLUDES = $(all_includes)
