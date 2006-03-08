kde4_header()

add_subdirectory( interfaces ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(gpgme_LIB_SRCS 
	context.cpp key.cpp 
	trustitem.cpp 
	data.cpp 
	callbacks.cpp 
	eventloopinteractor.cpp 
	keylistresult.cpp            
	keygenerationresult.cpp 
	importresult.cpp
	decryptionresult.cpp 
	verificationresult.cpp 
	signingresult.cpp 
	encryptionresult.cpp 
	engineinfo.cpp
)


kde4_automoc(${gpgme_LIB_SRCS})

kde4_add_library(gpgme++ SHARED ${gpgme_LIB_SRCS})

target_link_libraries(gpgme++ ${QT_AND_KDECORE_LIBS} )

set_target_properties(gpgme++ PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} gpgme++ )


########### install files ###############

install_files( /include/gpgme++ FILES  )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#KDE_OPTIONS = foreign
##AM_CXXFLAGS = -Wno-deprecated-declarations
#
#SUBDIRS = interfaces .
#
#INCLUDES =  -I$(srcdir) -I$(top_srcdir) \
#	$(GPGME_CFLAGS) $(all_includes)
#
#gpgmeppdir = $(includedir)/gpgme++
#gpgmepp_HEADERS = context.h \
#		key.h \
#		trustitem.h \
#		eventloopinteractor.h \
#		data.h \
#		gpgmefw.h \
#		result.h \
#		keylistresult.h \
#		keygenerationresult.h \
#		importresult.h \
#		decryptionresult.h \
#		verificationresult.h \
#		signingresult.h \
#		encryptionresult.h \
#		engineinfo.h
#
#noinst_HEADERS = context_p.h data_p.h shared.h callbacks.h gpgme-0-3-compat.h result_p.h util.h
#
#lib_LTLIBRARIES = libgpgme++.la
#libgpgme___la_SOURCES = context.cpp \
#			key.cpp \
#			trustitem.cpp \
#			data.cpp \
#			callbacks.cpp \
#			eventloopinteractor.cpp \
#			keylistresult.cpp \
#			keygenerationresult.cpp \
#			importresult.cpp \
#			decryptionresult.cpp \
#			verificationresult.cpp \
#			signingresult.cpp \
#			encryptionresult.cpp \
#			engineinfo.cpp
#
## --version-info CURRENT:REVISION:AGE
##   (Code changed:                      REVISION++)
##   (Interfaces added/removed/changed:  CURRENT++, REVISION=0)
##   (Interfaces added:                  AGE++)
##   (Interfaces removed/changed:        AGE=0)
#libgpgme___la_LDFLAGS = -no-undefined -version-info 2:0:2
#libgpgme___la_LIBADD = $(GPGME_LIBS)
#libgpgme___la_DEPENDENCIES = $(GPGME_LIBS_DEP)
#
#include $(top_srcdir)/admin/Doxyfile.am
