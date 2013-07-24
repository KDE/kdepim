project(akonadiconsole)

include_directories(
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_CORE_INCLUDE_DIR}
  ${NEPOMUK_WIDGETS_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${CMAKE_SOURCE_DIR}/messageviewer/
)

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

set(akonadiconsole_bin_SRCS
  agentwidget.cpp
  agentconfigdialog.cpp
  agentconfigmodel.cpp
  akonadibrowsermodel.cpp
  browserwidget.cpp
  collectionattributespage.cpp
  collectioninternalspage.cpp
  collectionaclpage.cpp
  connectionpage.cpp
  dbaccess.cpp
  dbbrowser.cpp
  dbconsole.cpp
  debugwidget.cpp
  instanceselector.cpp
  main.cpp
  mainwidget.cpp
  mainwindow.cpp
  monitorswidget.cpp
  monitorsmodel.cpp
  monitoritem.cpp
  notificationmodel.cpp
  notificationmonitor.cpp
  rawsocketconsole.cpp
  searchdialog.cpp
  searchwidget.cpp
  jobtracker.cpp
  jobtrackerwidget.cpp
  jobtrackermodel.cpp
)

qt4_generate_dbus_interface(jobtracker.h org.freedesktop.Akonadi.JobTracker.xml)
qt4_add_dbus_adaptor(akonadiconsole_bin_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Akonadi.JobTracker.xml jobtracker.h JobTracker)

qt4_add_dbus_interfaces(akonadiconsole_bin_SRCS
  ${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.TracerNotification.xml
)
# Use intalled interface once we can depend on Akoandi 1.1
set_source_files_properties(org.freedesktop.Akonadi.DebugInterface.xml PROPERTIES CLASSNAME DebugInterface)
qt4_add_dbus_interface(akonadiconsole_bin_SRCS
  org.freedesktop.Akonadi.DebugInterface.xml
  debuginterface
)

set_source_files_properties(${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.NotificationManager.xml
  PROPERTIES INCLUDE "akonadi/private/notificationmessage_p.h")

set_source_files_properties(${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.NotificationSource.xml
  PROPERTIES INCLUDE "metatypes.h")

qt4_add_dbus_interface(akonadiconsole_bin_SRCS
  ${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.NotificationManager.xml
  notificationmanagerinterface
)

qt4_add_dbus_interface(akonadiconsole_bin_SRCS
  ${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.NotificationSource.xml
  notificationsourceinterface
)

kde4_add_ui_files(akonadiconsole_bin_SRCS
  agentwidget.ui
  agentconfigdialog.ui
  browserwidget_itemview.ui
  collectionattributespage.ui
  collectionaclpage.ui
  dbbrowser.ui
  dbconsole.ui
  instanceselector.ui
  rawsocketconsole.ui
)

if(${KDE_VERSION} VERSION_GREATER "4.5.74")
  set(KDE_IS_AT_LEAST_4_5_74 TRUE)
endif()

if(KDE_IS_AT_LEAST_4_5_74)
  kde4_add_ui_files(akonadiconsole_bin_SRCS
    browserwidget_contentview.ui
    collectioninternalspage.ui
  )
else()
  kde4_add_ui_files(akonadiconsole_bin_SRCS
    browserwidget_contentview-45.ui
    collectioninternalspage-45.ui
  )
endif()

kde4_add_kcfg_files(akonadiconsole_bin_SRCS settings.kcfgc)

kde4_add_app_icon(akonadiconsole_bin_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-app-akonadiconsole.png")

kde4_add_executable(akonadiconsole_bin ${akonadiconsole_bin_SRCS})
set_target_properties(akonadiconsole_bin PROPERTIES OUTPUT_NAME akonadiconsole)

target_link_libraries(akonadiconsole_bin
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_AKONADI_CONTACT_LIBS}
  akonadi_next
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  calendarsupport
  ${KDEPIMLIBS_KCALCORE_LIBS}
  kdepim
  messageviewer
  ${KDE4_KCMUTILS_LIBS}
  ${KDE4_KIO_LIBS}
  ${QT_QTSQL_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${AKONADI_COMMON_LIBRARIES}
  ${NEPOMUK_CORE_LIBRARY}
  ${NEPOMUK_WIDGETS_LIBRARY}
)

install(TARGETS akonadiconsole_bin ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS akonadiconsole.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES akonadiconsoleui.rc DESTINATION ${DATA_INSTALL_DIR}/akonadiconsole)

add_subdirectory(icons)

