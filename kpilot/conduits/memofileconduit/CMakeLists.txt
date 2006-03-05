kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(conduit_memofile_PART_SRCS 
   memofile-factory.cc 
   memofile.cc 
   memofiles.cc 
   memofile-conduit.cc )

kde4_automoc(${conduit_memofile_PART_SRCS})

kde4_add_ui_files(conduit_memofile_PART_SRCS setup_base.ui )

kde4_add_kcfg_files(conduit_memofile_PART_SRCS memofileSettings.kcfgc )

kde4_add_plugin(conduit_memofile ${conduit_memofile_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_memofile )

target_link_libraries(conduit_memofile  ${QT_AND_KDECORE_LIBS} kpilot kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_memofile )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES memofileconduit.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES memofile-conduit.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = memofile-conduit.desktop
#
#kde_module_LTLIBRARIES = conduit_memofile.la
#
#
#conduit_memofile_la_SOURCES = memofileSettings.kcfgc setup_base.ui \
#	memofile-factory.cc memofile.cc memofiles.cc memofile-conduit.cc
#conduit_memofile_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#conduit_memofile_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI) $(LIB_KFILE)
#
#kde_kcfg_DATA = memofileconduit.kcfg
