#disable for now - it doesn't link
#add_subdirectory( akonadi )
add_subdirectory( icons )

IF (USE_CRASHHANDLER)
	SET(crashhandler_SRCS crashhandler.cpp )
ELSE (USE_CRASHHANDLER)
	SET(crashhandler_SRCS "" )
ENDIF (USE_CRASHHANDLER)

########### next target ###############

set(kmobiletoolslib_LIB_SRCS
   ifaces/addressbook.h
   ifaces/addressbook.cpp
   ifaces/dialaction.h
   ifaces/dialaction.cpp
   ifaces/shortmessages.h
   ifaces/shortmessages.cpp
   ifaces/smsfolders.h
   ifaces/smsfolders.cpp
   ifaces/status.h
   ifaces/status.cpp
   ifaces/information.h
   ifaces/information.cpp
   ifaces/guiservice.h
   ifaces/guiservice.cpp
   ifaces/actionprovider.h
   ifaces/actionprovider.cpp
   ifaces/wizardprovider.h
   ifaces/wizardprovider.cpp
   ifaces/jobprovider.h
   ifaces/jobprovider.cpp
   enginexp.h
   enginexp.cpp
   coreservice.h
   coreservice.cpp
   deviceloader.h
   deviceloader.cpp
   serviceloader.h
   serviceloader.cpp
   jobxp.h
   jobxp.cpp
   jobmanager.h
   jobmanager.cpp
   config.h
   config.cpp
   addressbook.h
   addressbook.cpp
   addressbookentry.h
   addressbookentry.cpp
   aboutdata.h
   aboutdata.cpp
   ${crashhandler_SRCS}
   #contactslist.cpp  
   #devicemenu.cpp 
   encodingshelper.h
   encodingshelper.cpp
   #engineslist.cpp 
   #homepage.cpp 
   #devicesconfig.cpp
   errorlog.h
   errorlog.cpp
   errorhandler.h
   errorhandler.cpp
   errortypes/baseerror.h
   errortypes/baseerror.cpp
#   kmobiletools_devices_addons.cpp 
   kmobiletoolshelper.h
   kmobiletoolshelper.cpp 
   overlaywidget.h
   overlaywidget.cpp 
   job.h
   job.cpp 
   weaver.h
   weaver.cpp 
   #enginedata.cpp
   #qserial.cpp 
   #picksmscenter.cpp
   #serialdevice.cpp 
   #engine.cpp 
   sms.h
   sms.cpp 
   smslist.h
   smslist.cpp )

QT4_ADD_DBUS_INTERFACE(kmobiletoolslib_LIB_SRCS org.kde.kmobiletools.device.xml kmobiletools_device_interface )
QT4_ADD_DBUS_INTERFACE(kmobiletoolslib_LIB_SRCS org.kde.kmobiletools.main.xml kmobiletools_main_interface )



kde4_add_kcfg_files(kmobiletoolslib_LIB_SRCS kmobiletools_cfg.kcfgc kmobiletools_devices.kcfgc )

kde4_add_library(kmobiletoolslib SHARED ${kmobiletoolslib_LIB_SRCS})

target_link_libraries(kmobiletoolslib
   ${KDE4_KDECORE_LIBS}
   ${KDE4_KDEUI_LIBS}
   ${KDE4_KIO_LIBS}  
   ${KDE4_KHTML_LIBRARY}
   ${KDE4_THREADWEAVER_LIBRARY}
   ${KDE4_KABC_LIBRARY} 
   ${KDE4_KCAL_LIBRARY}
   ${KDE4_KMIME_LIBS}
   ${QT_QT3SUPPORT_LIBRARY} )

set_target_properties(kmobiletoolslib PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kmobiletoolslib ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############
install( FILES kmobiletools_devices.kcfg kmobiletools_devices.kcfg DESTINATION  ${KCFG_INSTALL_DIR} )


install( FILES  kmobiletoolsengine.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install( FILES  kmobiletoolsenginexp.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install( FILES  kmobiletoolscoreservice.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )

# install interfaces
install(FILES
   ifaces/addressbook.h
   ifaces/dialaction.h
   ifaces/shortmessages.h
   ifaces/smsfolders.h
   ifaces/status.h
   ifaces/information.h
   ifaces/guiservice.h
   ifaces/actionprovider.h
   ifaces/wizardprovider.h
   ifaces/jobprovider.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/libkmobiletools/ifaces)

# install error types
install(FILES
   errortypes/baseerror.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/libkmobiletools/errortypes)

install(FILES
   status.h
   information.h
   enginexp.h
   coreservice.h
   config.h
   addressbook.h
   addressbookentry.h
   kmobiletools_export.h
   picksmscenter.h
   kmobiletoolshelper.h
   errorlog.h
   errorhandler.h
   errortypes/baseerror.h
   deviceloader.h
   serviceloader.h
   engine.h
   sms.h
   smslist.h
   encodingshelper.h
   gsmcodec.h
   qserial.h
   job.h
   contactslist.h
   serialdevice.h
   enginedata.h
   weaver.h
   jobxp.h
   jobmanager.h
   ${CMAKE_CURRENT_BINARY_DIR}/kmobiletools_devices.h
   devicesconfig.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/libkmobiletools)

