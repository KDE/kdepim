project(kleopatra)

set(libconf_SRCS  ${CMAKE_SOURCE_DIR}/kleopatra/conf/configuredialog.cpp )
add_subdirectory( conf ) 
add_subdirectory( kwatchgnupg ) 

include_directories( ${CMAKE_SOURCE_DIR}/libkleo )

########### next target ###############

set(kleopatra_bin_SRCS ${libconf_SRCS}
	customactions.cpp 
   aboutdata.cpp 
   main.cpp 
   certmanager.cpp 
   hierarchyanalyser.cpp 
   certificatewizardimpl.cpp 
   certificateinfowidgetimpl.cpp 
   crlview.cpp 
   certlistview.cpp )

kde4_automoc(${kleopatra_bin_SRCS})

kde4_add_ui3_files(kleopatra_bin_SRCS certificatewizard.ui certificateinfowidget.ui )

kde4_add_executable(kleopatra_bin ${kleopatra_bin_SRCS})
set_target_properties(kleopatra_bin  PROPERTIES OUTPUT_NAME kleopatra)

target_link_libraries(kleopatra_bin ${KDE4_KDE3SUPPORT_LIBS} kleopatra kabc ${KDE4_KUTILS_LIBS} )

install(TARGETS kleopatra_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kleopatra_import.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES kleopatraui.rc  DESTINATION ${DATA_INSTALL_DIR}/kleopatra)




#original Makefile.am contents follow:

#SUBDIRS = conf kwatchgnupg .
#
#KDE_CXXFLAGS = -DLIBKLEOPATRA_NO_COMPAT
#
#INCLUDES = -I$(top_srcdir)/libkleo -I$(top_srcdir) $(GPGME_CFLAGS) $(all_includes)
#
#bin_PROGRAMS = kleopatra
#
#kleopatra_SOURCES = \
#	customactions.cpp \
#	aboutdata.cpp \
#	main.cpp certmanager.cpp \
#	hierarchyanalyser.cpp \
#	certificatewizard.ui certificatewizardimpl.cpp \
#	certificateinfowidget.ui certificateinfowidgetimpl.cpp \
#	crlview.cpp certlistview.cpp
#
#METASOURCES = AUTO
#
#kleopatra_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kleopatra_LDADD   = conf/libconf.la ../libkleo/libkleopatra.la $(LIB_KUTILS) $(LIB_KABC)
#
#rcdir = $(kde_datadir)/kleopatra
#rc_DATA = kleopatraui.rc
#
#xdg_apps_DATA = kleopatra_import.desktop
#
#messages: rc.cpp
#	$(EXTRACTRC) conf/*.ui >> rc.cpp
#	$(XGETTEXT) conf/*.cpp *.cpp *.h -o $(podir)/kleopatra.pot
#
#include $(top_srcdir)/admin/Doxyfile.am
