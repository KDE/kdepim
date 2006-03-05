kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kpilot/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(conduit_null_PART_SRCS null-conduit.cc null-factory.cc )

kde4_automoc(${conduit_null_PART_SRCS})

kde4_add_ui_files(conduit_null_PART_SRCS setup_base.ui )

kde4_add_kcfg_files(conduit_null_PART_SRCS nullSettings.kcfgc )

kde4_add_plugin(conduit_null ${conduit_null_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} conduit_null )

target_link_libraries(conduit_null  ${QT_AND_KDECORE_LIBS} kpilot kdeui )

install_targets(${LIB_INSTALL_DIR}/kde4 conduit_null )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES nullconduit.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES null-conduit.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = null-conduit.desktop
#
#kde_module_LTLIBRARIES = conduit_null.la
#
#
#conduit_null_la_SOURCES = nullSettings.kcfgc setup_base.ui null-conduit.cc null-factory.cc
#conduit_null_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#conduit_null_la_LIBADD = ../../lib/libkpilot.la $(LIB_KDEUI)
#
#kde_kcfg_DATA = nullconduit.kcfg
