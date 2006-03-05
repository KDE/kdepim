kde4_header()

add_subdirectory( conf ) 
add_subdirectory( kwatchgnupg ) 

include_directories( ${CMAKE_SOURCE_DIR}/libkleo ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kleopatra_SRCS 
   customactions.cpp 
   aboutdata.cpp 
   main.cpp 
   certmanager.cpp 
   hierarchyanalyser.cpp 
   certificatewizardimpl.cpp 
   certificateinfowidgetimpl.cpp 
   crlview.cpp 
   certlistview.cpp )

kde4_automoc(${kleopatra_SRCS})

kde4_add_ui3_files(kleopatra_SRCS certificatewizard.ui certificateinfowidget.ui )

kde4_add_executable(kleopatra ${kleopatra_SRCS})

target_link_libraries(kleopatra  ${QT_AND_KDECORE_LIBS} conf kleopatra kutils )

install_targets(/bin kleopatra )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kleopatra_import.desktop )
install_files( ${DATA_INSTALL_DIR}/kleopatra FILES kleopatraui.rc )

kde4_create_doxygen_docs(  )

kde4_footer()



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
