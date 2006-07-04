



########### next target ###############

set(libnetworkstatus_SRCS networkstatuscommon.cpp clientadaptor.cpp provideradaptor.cpp serviceadaptor.cpp)
qt4_add_dbus_adaptor( libnetworkstatus_SRCS org.kde.networkstatus.client.xml 
    networkstatus.h NetworkStatusModule )
qt4_add_dbus_adaptor( libnetworkstatus_SRCS org.kde.networkstatus.provider.xml 
    testservice.h TestService )
qt4_add_dbus_adaptor( libnetworkstatus_SRCS org.kde.networkstatus.service.xml
    networkstatus.h NetworkStatusModule )

qt4_add_dbus_interfaces( libnetworkstatus_SRCS org.kde.networkstatus.client.xml )
qt4_add_dbus_interfaces( libnetworkstatus_SRCS org.kde.networkstatus.provider.xml )
qt4_add_dbus_interfaces( libnetworkstatus_SRCS org.kde.networkstatus.service.xml )


set(kded_networkstatus_PART_SRCS networkstatus.cpp network.cpp ${libnetworkstatus_SRCS})

kde4_automoc(${kded_networkstatus_PART_SRCS})

kde4_add_plugin(kded_networkstatus ${kded_networkstatus_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kded_networkstatus )

target_link_libraries(kded_networkstatus  ${KDE4_KIO_LIBS}  )

install(TARGETS kded_networkstatus  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############


########### next target ###############


########### next target ###############

set(networkstatustestservice_SRCS testservice.cpp ${libnetworkstatus_SRCS} )

kde4_automoc(${networkstatustestservice_SRCS})


kde4_add_executable(networkstatustestservice ${networkstatustestservice_SRCS})

target_link_libraries(networkstatustestservice  ${KDE4_KIO_LIBS}  )

install(TARGETS networkstatustestservice  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES networkstatus.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kded)




#original Makefile.am contents follow:

##SUBDIRS = networkstatustray
#
#METASOURCES = AUTO
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#kde_module_LTLIBRARIES = kded_networkstatus.la 
#noinst_LTLIBRARIES = libnetworkstatus.la libconnectionmanager.la
#
#libnetworkstatus_la_LIBADD = $(LIB_KDECORE)
#libnetworkstatus_la_LDFLAGS = $(all_libraries)
#libnetworkstatus_la_SOURCES = networkstatuscommon.cpp
#
#libconnectionmanager_la_LIBADD = $(LIB_KDECORE)
#libconnectionmanager_la_LDFLAGS = $(all_libraries)
#libconnectionmanager_la_SOURCES = connectionmanager.cpp connectionmanager.skel clientiface.stub
#
#INCLUDES = $(all_includes)
#
#kded_networkstatus_la_SOURCES = networkstatus.cpp networkstatus.skel \
#	clientiface.skel serviceiface.skel network.cpp
##kclientiface.skel serviceiface.skel
#kded_networkstatus_la_LIBADD = $(LIB_KDECORE) $(LIB_KIO) ./libnetworkstatus.la
#kded_networkstatus_la_LDFLAGS = $(all_libraries) -module -avoid-version 
#
#servicesdir = $(kde_servicesdir)/kded
#services_DATA = networkstatus.desktop
#
#noinst_HEADERS = serviceiface.h networkstatuscommon.h serviceifaceimpl.h \
#	network.h clientifaceimpl.h provideriface.h testservice.h connectionmanager.h
#
#bin_PROGRAMS = networkstatustestservice
#
#networkstatustestservice_LDFLAGS = $(all_libraries)
#networkstatustestservice_LDADD = libnetworkstatus.la
#
#networkstatustestservice_SOURCES = testservice.cpp provideriface.skel serviceiface.stub
