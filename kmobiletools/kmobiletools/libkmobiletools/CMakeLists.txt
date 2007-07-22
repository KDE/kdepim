add_subdirectory( akonadi )
add_subdirectory( icons )

IF (USE_CRASHHANDLER)
	SET(crashhandler_SRCS crashhandler.cpp )
ELSE (USE_CRASHHANDLER)
	SET(crashhandler_SRCS "" )
ENDIF (USE_CRASHHANDLER)

########### next target ###############

set(kmobiletools_LIB_SRCS
   ifaces/addressbook.cpp
   ifaces/dialaction.cpp
   ifaces/shortmessages.cpp
   ifaces/smsfolders.cpp
   ifaces/status.cpp
   aboutdata.cpp 
   ${crashhandler_SRCS}
   contactslist.cpp  
   devicemenu.cpp 
   encodingshelper.cpp 
   engineslist.cpp 
   homepage.cpp 
   devicesconfig.cpp
   errorlog.cpp
   errorhandler.cpp
   errortypes/baseerror.cpp
#   kmobiletools_devices_addons.cpp 
   kmobiletoolshelper.cpp 
   overlaywidget.cpp 
   job.cpp 
   weaver.cpp 
   enginedata.cpp
   qserial.cpp 
   picksmscenter.cpp
   serialdevice.cpp 
   engine.cpp 
   sms.cpp 
   smslist.cpp )

QT4_ADD_DBUS_INTERFACE(kmobiletools_LIB_SRCS org.kde.kmobiletools.device.xml kmobiletools_device_interface )
QT4_ADD_DBUS_INTERFACE(kmobiletools_LIB_SRCS org.kde.kmobiletools.main.xml kmobiletools_main_interface )



kde4_add_kcfg_files(kmobiletools_LIB_SRCS kmobiletools_cfg.kcfgc kmobiletools_devices.kcfgc )

kde4_add_library(kmobiletools SHARED ${kmobiletools_LIB_SRCS})

target_link_libraries(kmobiletools ${KDE4_KDECORE_LIBS} ${KDE4_KHTML_LIBRARY} ${KDE4_THREADWEAVER_LIBRARY} ${KDE4_KABC_LIBRARY} ${KDE4_KCAL_LIBRARY} ${KDE4_KMIME_LIBS} akonadi)

set_target_properties(kmobiletools PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kmobiletools DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############
install( FILES kmobiletools_devices.kcfg kmobiletools_devices.kcfg DESTINATION  ${KCFG_INSTALL_DIR} )


install( FILES  kmobiletoolsengine.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install(FILES
   ifaces/addressbook.h
   ifaces/dialaction.h
   ifaces/shortmessages.h
   ifaces/smsfolders.h
   ifaces/status.h
   addressbook.h
   kmobiletools_export.h
   picksmscenter.h
   kmobiletoolshelper.h
   errorlog.h
   errorhandler.h
   errortypes/baseerror.h
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
   ${CMAKE_CURRENT_BINARY_DIR}/kmobiletools_devices.h
   devicesconfig.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/libkmobiletools)

