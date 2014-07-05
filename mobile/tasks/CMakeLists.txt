project(task-mobile)

include_directories(
  ${Boost_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/calendarsupport/
  ${CMAKE_BINARY_DIR}/calendarsupport/
  ${CMAKE_SOURCE_DIR}/calendarviews
)



set(mobile_tasks_SRCS
  configwidget.cpp
  searchwidget.cpp
  tasklistproxy.cpp
  tasksfilterproxymodel.cpp
  tasksexporthandler.cpp
  tasksimporthandler.cpp
  tasksactionmanager.cpp
  taskthreadgroupercomparator.cpp
  mainview.cpp
  main.cpp
)

ki18n_wrap_ui(mobile_tasks_SRCS configwidget.ui searchwidget.ui)
kconfig_add_kcfg_files(mobile_tasks_SRCS settings.kcfgc)

if (RUNTIME_PLUGINS_STATIC)
  add_definitions(-DKCALCORE_SERIALIZER_PLUGIN_STATIC)
endif ()

kde4_add_app_icon(mobile_tasks_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-apps-tasks-mobile.png")

add_executable(tasks-mobile ${mobile_tasks_SRCS} )

target_link_libraries(tasks-mobile
  Qt5::Declarative
  KF5::CalendarCore
  kdepimdbusinterfaces
  kdepimmobileui
  kdepimmobileuicalendar
  kdepim
  eventviews
)

if (RUNTIME_PLUGINS_STATIC)
  target_link_libraries(tasks-mobile akonadi_serializer_addressee akonadi_serializer_contactgroup akonadi_serializer_kcalcore akonadi_serializer_mail)
endif ()

install(TARGETS tasks-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS tasks-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  BulkActionComponent.qml
  ConfigDialog.qml
  tasks.qml
  tasks-mobile-background.png
  TaskActions.qml
  TaskListView.qml
  DESTINATION ${DATA_INSTALL_DIR}/tasks-mobile
)
ecm_install_icons(ICONS hi48-apps-tasks-mobile.png  hi64-apps-tasks-mobile.png  hi80-apps-tasks-mobile-harmattan.png DESTINATION ${ICON_INSTALL_DIR} )
