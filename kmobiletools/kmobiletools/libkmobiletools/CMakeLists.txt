add_subdirectory( errortypes )
add_subdirectory( akonadi )
add_subdirectory( icons )

IF (USE_CRASHHANDLER)
	SET(crashhandler_SRCS crashhandler.cpp )
ELSE (USE_CRASHHANDLER)
	SET(crashhandler_SRCS "" )
ENDIF (USE_CRASHHANDLER)

########### next target ###############

set(kmobiletools_LIB_SRCS 
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


kde4_automoc(${kmobiletools_LIB_SRCS})

kde4_add_kcfg_files(kmobiletools_LIB_SRCS kmobiletools_cfg.kcfgc kmobiletools_devices.kcfgc )

kde4_add_library(kmobiletools SHARED ${kmobiletools_LIB_SRCS})

target_link_libraries(kmobiletools ${KDE4_KDECORE_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_THREADWEAVER_LIBRARY} ${KDE4_KABC_LIBS} ${KDE4_KCAL_LIBS} ${KDE4_KMIME_LIBS} akonadi)

set_target_properties(kmobiletools PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kmobiletools DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############
install( FILES kmobiletools_devices.kcfg kmobiletools_devices.kcfg DESTINATION  ${KCFG_INSTALL_DIR} )


install( FILES  kmobiletoolsengine.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install(FILES
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

