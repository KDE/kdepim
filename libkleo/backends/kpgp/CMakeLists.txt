kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${CMAKE_SOURCE_DIR}/libkpgp  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES =  \
#	-I$(top_srcdir)/libkleo \
#	-I$(top_srcdir)/libkpgp \
#	$(GPGME_CFLAGS) $(all_includes)
#
#noinst_LTLIBRARIES = libkleopatra_backend_kpgp.la
#
#libkleopatra_backend_kpgp_la_SOURCES = \
#			kpgpbackendbase.cpp kpgpwrapper.cpp \
#                        kpgpkeylistjob.cpp
#
#METASOURCES = AUTO
