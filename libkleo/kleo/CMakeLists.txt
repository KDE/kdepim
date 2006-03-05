kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############

install_files( /include/kleo FILES  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) \
#        -I$(top_srcdir)/libkleo \
#	$(GPGME_CFLAGS) $(all_includes)
#
#noinst_LTLIBRARIES = libkleopatra_core.la
#
#libkleopatra_core_la_SOURCES = \
#	cryptobackend.cpp \
#	cryptobackendfactory.cpp \
#	enum.cpp \
#	dn.cpp \
#	job.cpp \
#	\
#	multideletejob.cpp \
#	hierarchicalkeylistjob.cpp \
#	\
#	kconfigbasedkeyfilter.cpp \
#	keyfiltermanager.cpp
#
#kleodir = $(includedir)/kleo
#kleo_HEADERS = \
#	enum.h \
#	cryptobackend.h \
#	cryptobackendfactory.h \
#	cryptoconfig.h \
#	dn.h \
#	job.h \
#	keylistjob.h keygenerationjob.h importjob.h exportjob.h downloadjob.h deletejob.h \
#	encryptjob.h decryptjob.h signjob.h verifydetachedjob.h verifyopaquejob.h \
#	decryptverifyjob.h signencryptjob.h refreshkeysjob.h \
#	multideletejob.h \
#	hierarchicalkeylistjob.h \
#	\
#	keyfilter.h \
#	keyfiltermanager.h
#
#METASOURCES = AUTO
