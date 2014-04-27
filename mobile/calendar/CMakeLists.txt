project(korganizer-mobile)

include_directories(
  # This is probably not the correct way to get KCalPrefs in.
  ${CMAKE_SOURCE_DIR}/calendarsupport/
  ${CMAKE_BINARY_DIR}/calendarsupport/
  ${CMAKE_SOURCE_DIR}/calendarviews
  ${KDEPIMLIBS_INCLUDE_DIR}
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)

kde_enable_exceptions()

set(korganizer_mobile_SRCS
  agendaviewitem.cpp
  calendarinterface.cpp
  configwidget.cpp
  declarativewidgetwrapper.cpp
  eventlistproxy.cpp
  eventsfilterproxymodel.cpp
  eventsguistatemanager.cpp
  eventsexporthandler.cpp
  eventsimporthandler.cpp
  mainview.cpp
  main.cpp
  monthviewitem.cpp
  searchwidget.cpp
  timelineviewitem.cpp
  qmldateedit.cpp
)

qt5_wrap_ui(korganizer_mobile_SRCS configwidget.ui searchwidget.ui)
kconfig_add_kcfg_files(korganizer_mobile_SRCS settings.kcfgc)

qt5_add_dbus_adaptor(korganizer_mobile_SRCS ${CMAKE_SOURCE_DIR}/korganizer/org.kde.Korganizer.Calendar.xml calendarinterface.h CalendarInterface )

if (RUNTIME_PLUGINS_STATIC)
  add_definitions(-DMAIL_SERIALIZER_PLUGIN_STATIC)
endif ()

kde4_add_app_icon(korganizer_mobile_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-apps-korganizer-mobile.png")

add_executable(korganizer-mobile ${korganizer_mobile_SRCS} )

target_link_libraries(korganizer-mobile
  Qt5::Declarative
  
  
  ${KDE4_KDECLARATIVE_LIBS}
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::Holidays
  KF5::AkonadiCore
  ${KDEPIMLIBS_AKONADI_CALENDAR_LIBS}
  kdepimdbusinterfaces
  kdepimmobileui
  kdepimmobileuicalendar
  incidenceeditorsngmobile
  eventviews
  calendarsupport
  kdepim
)

if (RUNTIME_PLUGINS_STATIC)
  target_link_libraries(korganizer-mobile akonadi_serializer_addressee akonadi_serializer_contactgroup akonadi_serializer_kcalcore akonadi_serializer_mail)
endif ()

install(TARGETS korganizer-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS korganizer-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  BulkActionComponent.qml
  ConfigDialog.qml
  korganizer-mobile.qml
  KorganizerActions.qml
  korganizer-mobile-background.png
  EventListView.qml
  EventListComponent.qml
  MonthViewComponent.qml
  TimelineView.qml
  DESTINATION ${DATA_INSTALL_DIR}/korganizer-mobile
)
ecm_install_icons( ${ICON_INSTALL_DIR} )
