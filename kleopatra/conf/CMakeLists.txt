kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### next target ###############

set(kcm_kleopatra_PART_SRCS 
   dirservconfigpage.cpp 
   appearanceconfigpage.cpp 
   appearanceconfigwidget.cpp 
   dnorderconfigpage.cpp )

kde4_automoc(${kcm_kleopatra_PART_SRCS})

kde4_add_ui_files(kcm_kleopatra_PART_SRCS appearanceconfigwidgetbase.ui )

kde4_add_plugin(kcm_kleopatra ${kcm_kleopatra_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kleopatra )

target_link_libraries(kcm_kleopatra  ${QT_AND_KDECORE_LIBS} kleopatra kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kleopatra )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kleopatra_config_dirserv.desktop  	kleopatra_config_appear.desktop  	kleopatra_config_dnorder.desktop )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -DLIBKLEOPATRA_NO_COMPAT -I$(top_srcdir)/libkleo -I$(top_srcdir)/libkleo/ui \
#   -I$(top_builddir)/libkleo/ui $(GPGME_CFLAGS) $(all_includes)
#
#METASOURCES = AUTO
#
## the stuff that gets compiled into kleopatra
#noinst_LTLIBRARIES = libconf.la
#libconf_la_SOURCES = configuredialog.cpp
#
#kde_module_LTLIBRARIES = kcm_kleopatra.la
#
#kcm_kleopatra_la_SOURCES = dirservconfigpage.cpp \
#	appearanceconfigpage.cpp \
#	appearanceconfigwidgetbase.ui \
#	appearanceconfigwidget.cpp \
#	dnorderconfigpage.cpp
#
#kcm_kleopatra_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_kleopatra_la_LIBADD = ../../libkleo/libkleopatra.la $(LIB_KDEUI)
#
#kde_services_DATA = \
#	kleopatra_config_dirserv.desktop \
#	kleopatra_config_appear.desktop \
#	kleopatra_config_dnorder.desktop
