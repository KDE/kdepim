kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##KDE_OPTIONS = nofinal
#
#INCLUDES =  -I$(top_srcdir) \
#	-I$(top_srcdir)/libkleo \
#	$(GPGME_CFLAGS) $(all_includes)
#
#noinst_LTLIBRARIES = libkleopatra_backend_qgpgme.la
#
#libkleopatra_backend_qgpgme_la_SOURCES = \
#			gnupgprocessbase.cpp \
#			qgpgmeprogresstokenmapper.cpp \
#			\
#			qgpgmebackend.cpp \
#			\
#			qgpgmejob.cpp \
#			\
#			qgpgmekeylistjob.cpp \
#			qgpgmekeygenerationjob.cpp \
#			qgpgmeimportjob.cpp \
#			qgpgmeexportjob.cpp \
#			qgpgmesecretkeyexportjob.cpp \
#			qgpgmedownloadjob.cpp \
#			qgpgmedeletejob.cpp \
#			qgpgmeencryptjob.cpp \
#			qgpgmedecryptjob.cpp \
#			qgpgmesignjob.cpp \
#			qgpgmeverifydetachedjob.cpp \
#			qgpgmeverifyopaquejob.cpp \
#			qgpgmesignencryptjob.cpp \
#			qgpgmedecryptverifyjob.cpp \
#			qgpgmecryptoconfig.cpp \
#			qgpgmerefreshkeysjob.cpp
#
#METASOURCES = AUTO
