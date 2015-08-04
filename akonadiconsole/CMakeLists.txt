project(akonadiconsole)

include_directories(
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


set(akonadiconsole_bin_SRCS
  akonadiconsole_debug.cpp
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
  searchdialog.cpp
  searchwidget.cpp
  querydebugger.cpp
  jobtracker.cpp
  jobtrackerwidget.cpp
  jobtrackermodel.cpp
  tagpropertiesdialog.cpp
)

qt5_generate_dbus_interface(jobtracker.h org.freedesktop.Akonadi.JobTracker.xml)
qt5_add_dbus_adaptor(akonadiconsole_bin_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Akonadi.JobTracker.xml jobtracker.h JobTracker)

qt5_add_dbus_interfaces(akonadiconsole_bin_SRCS
  ${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.TracerNotification.xml
)
# Use intalled interface once we can depend on Akoandi 1.1
set_source_files_properties(org.freedesktop.Akonadi.DebugInterface.xml PROPERTIES CLASSNAME DebugInterface)
qt5_add_dbus_interface(akonadiconsole_bin_SRCS
  org.freedesktop.Akonadi.DebugInterface.xml
  debuginterface
)

set_source_files_properties(${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.NotificationSource.xml
  PROPERTIES INCLUDE "akonadi/private/protocol_p.h"
)

qt5_add_dbus_interface(akonadiconsole_bin_SRCS
  ${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.NotificationManager.xml
  notificationmanagerinterface
)

qt5_add_dbus_interface(akonadiconsole_bin_SRCS
  ${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.NotificationSource.xml
  notificationsourceinterface
)

qt5_add_dbus_interface(akonadiconsole_bin_SRCS
  ${AKONADI_DBUS_INTERFACES_DIR}/org.freedesktop.Akonadi.StorageDebugger.xml
  storagedebuggerinterface
)

ki18n_wrap_ui(akonadiconsole_bin_SRCS
  agentwidget.ui
  agentconfigdialog.ui
  browserwidget_itemview.ui
  collectionattributespage.ui
  collectionaclpage.ui
  dbbrowser.ui
  dbconsole.ui
  instanceselector.ui
  browserwidget_contentview.ui
  collectioninternalspage.ui
  tagpropertiesdialog.ui
)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-akonadiconsole.png")
ecm_add_app_icon(akonadiconsole_bin_SRCS ICONS ${ICONS_SRCS})

add_executable(akonadiconsole_bin ${akonadiconsole_bin_SRCS})
set_target_properties(akonadiconsole_bin PROPERTIES OUTPUT_NAME akonadiconsole)

target_link_libraries(akonadiconsole_bin
  KF5::AkonadiCore
  KF5::AkonadiPrivate
  KF5::AkonadiContact
  calendarsupport
  KF5::CalendarCore
  kdepim
  messageviewer
  KF5::DBusAddons
  Qt5::Sql
)

install(TARGETS akonadiconsole_bin ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.akonadiconsole.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES akonadiconsoleui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/akonadiconsole)

add_subdirectory(icons)

