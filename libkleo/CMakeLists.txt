kde4_header()

add_subdirectory( pics ) 
add_subdirectory( kleo ) 
add_subdirectory( backends ) 
add_subdirectory( ui ) 
add_subdirectory( tests ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kleopatra_LIB_SRCS 
   cryptplug.cpp 
   cryptplugwrapper.cpp 
   cryptplugwrapperlist.cpp 
   cryptplugfactory.cpp )

kde4_automoc(${kleopatra_LIB_SRCS})

kde4_add_library(kleopatra SHARED ${kleopatra_LIB_SRCS})

target_link_libraries(kleopatra  ${QT_AND_KDECORE_LIBS} kleopatra_core kleopatra_backend_qgpgme kleopatra_backend_chiasmus kleopatra_ui qgpgme kio kdeui )

set_target_properties(kleopatra PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kleopatra )


########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir) $(GPGME_CFLAGS) $(all_includes)
#
#SUBDIRS = pics kleo backends ui . tests
#
#lib_LTLIBRARIES = libkleopatra.la
#
#libkleopatra_la_SOURCES = \
#			cryptplug.cpp \
#			cryptplugwrapper.cpp \
#			cryptplugwrapperlist.cpp \
#			cryptplugfactory.cpp
#
#libkleopatra_la_LIBADD = \
#			kleo/libkleopatra_core.la \
#			backends/qgpgme/libkleopatra_backend_qgpgme.la \
#			backends/chiasmus/libkleopatra_backend_chiasmus.la \
#			ui/libkleopatra_ui.la \
#			\
#			$(top_builddir)/qgpgme/libqgpgme.la \
#			$(LIB_KIO) $(LIB_KDEUI)
#
## Disabled for kdepim-3.3, see also kleo/cryptobackendfactory.cpp
## backends/kpgp/libkleopatra_backend_kpgp.la
#
## --version-info CURRENT:REVISION:AGE
##   (Code changed:                      REVISION++)
##   (Interfaces added/removed/changed:  CURRENT++, REVISION=0)
##   (Interfaces added:                  AGE++)
##   (Interfaces removed/changed:        AGE=0)
#libkleopatra_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
#
#METASOURCES = AUTO
#
## remove the .desktop suffix on install (which is needed for translation)
#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(kde_confdir)
#	$(INSTALL_DATA) $(srcdir)/libkleopatrarc.desktop $(DESTDIR)$(kde_confdir)/libkleopatrarc
#
#uninstall-local:
#	rm -f $(DESTDIR)$(kde_confdir)/libkleopatrarc
#
#messages: #rc.cpp
#	$(EXTRACTRC) */*.ui > rc.cpp
#	$(XGETTEXT) *.cpp *.h \
#	kleo/*.cpp kleo/*.h \
#	ui/*.cpp ui/*.h \
#	backends/*/*.cpp backends/*/*.h -o $(podir)/libkleopatra.pot
#
#include $(top_srcdir)/admin/Doxyfile.am
