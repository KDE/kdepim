project(task-mobile)

include_directories(
  ${KDEPIMLIBS_INCLUDE_DIR}
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/calendarsupport/
  ${CMAKE_BINARY_DIR}/calendarsupport/
  ${CMAKE_SOURCE_DIR}/calendarviews
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

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

qt4_wrap_ui(mobile_tasks_SRCS configwidget.ui searchwidget.ui)
kde4_add_kcfg_files(mobile_tasks_SRCS settings.kcfgc)

if (RUNTIME_PLUGINS_STATIC)
  add_definitions(-DKCALCORE_SERIALIZER_PLUGIN_STATIC)
endif ()

kde4_add_app_icon(mobile_tasks_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-apps-tasks-mobile.png")

kde4_add_executable(tasks-mobile ${mobile_tasks_SRCS} )
if (WINCE)
  SET_TARGET_PROPERTIES(tasks-mobile PROPERTIES LINK_FLAGS " /FIXED:NO")
endif ()

target_link_libraries(tasks-mobile
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_AKONADI_CALENDAR_LIBS}
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
kde4_install_icons( ${ICON_INSTALL_DIR} )
