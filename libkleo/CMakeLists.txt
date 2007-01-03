include_directories(${CMAKE_SOURCE_DIR}/libkpgp/ ${CMAKE_BINARY_DIR}/libkleo/backends/chiasmus/  ${GPGME_INCLUDES} )

add_subdirectory( pics )
add_subdirectory( kleo )
add_subdirectory( backends )
add_subdirectory( tests )



########### next target ###############
set(libkleopatra_backend_qgpgme_SRCS
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/gnupgprocessbase.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmeprogresstokenmapper.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmebackend.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmejob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmekeylistjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmekeygenerationjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmeimportjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmeexportjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmesecretkeyexportjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmedownloadjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmedeletejob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmeencryptjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmedecryptjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmesignjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmeverifydetachedjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmeverifyopaquejob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmesignencryptjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmedecryptverifyjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmecryptoconfig.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/qgpgme/qgpgmerefreshkeysjob.cpp
	)


set(libkleopatra_backend_chiasmus_SRCS 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/config_data.c 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/obtainkeysjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/symcryptrunprocessbase.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/chiasmusjob.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/chiasmuslibrary.cpp 
	${CMAKE_SOURCE_DIR}/libkleo/backends/chiasmus/chiasmusbackend.cpp
	)


set(libkleopatra_core_SRCS
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/cryptobackend.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/cryptobackendfactory.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/enum.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/dn.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/job.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/multideletejob.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/hierarchicalkeylistjob.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/kconfigbasedkeyfilter.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/kleo/keyfiltermanager.cpp
   )

set(libkleopatra_ui_SRCS
   ${CMAKE_SOURCE_DIR}/libkleo/ui/kdhorizontalline.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/progressbar.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/progressdialog.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/keylistview.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/keyselectiondialog.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/keyrequester.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/passphrasedialog.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/keyapprovaldialog.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/backendconfigwidget.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/dnattributeorderconfigwidget.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/cryptoconfigmodule.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/cryptoconfigdialog.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/directoryserviceswidget.cpp 
   ${CMAKE_SOURCE_DIR}/libkleo/ui/adddirectoryservicedialogimpl.cpp
)

kde4_add_ui_files(libkleopatra_ui_SRCS
	${CMAKE_SOURCE_DIR}/libkleo/ui/adddirectoryservicedialog.ui
	${CMAKE_SOURCE_DIR}/libkleo/ui/directoryserviceswidgetbase.ui
)

set(kleopatra_LIB_SRCS ${libkleopatra_core_SRCS} ${libkleopatra_ui_SRCS} ${libkleopatra_backend_chiasmus_SRCS} ${libkleopatra_backend_qgpgme_SRCS}
   cryptplug.cpp 
   cryptplugwrapper.cpp 
   cryptplugwrapperlist.cpp 
   cryptplugfactory.cpp )

kde4_automoc(${kleopatra_LIB_SRCS})

kde4_add_library(kleopatra SHARED ${kleopatra_LIB_SRCS})

target_link_libraries(kleopatra  ${KDE4_KDE3SUPPORT_LIBS} qgpgme  )

set_target_properties(kleopatra PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kleopatra  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############





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
