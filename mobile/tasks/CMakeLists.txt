project(task-mobile)

include_directories(
  ${KDEPIMLIBS_INCLUDE_DIR}
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(mobile_tasks_SRCS
  tasklistproxy.cpp
  mainview.cpp
  main.cpp
)

if (RUNTIME_PLUGINS_STATIC)
  add_definitions(-DKCALCORE_SERIALIZER_PLUGIN_STATIC)
endif (RUNTIME_PLUGINS_STATIC)

kde4_add_executable(tasks-mobile ${mobile_tasks_SRCS} )
if (WINCE)
  SET_TARGET_PROPERTIES(tasks-mobile PROPERTIES LINK_FLAGS " /FIXED:NO")
endif (WINCE)

target_link_libraries(tasks-mobile
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  kdepimdbusinterfaces
  kdepimmobileui
  kdepimmobileuicalendar
  kdepim
)

if (RUNTIME_PLUGINS_STATIC)
  target_link_libraries(tasks-mobile akonadi_serializer_kcalcore)
endif (RUNTIME_PLUGINS_STATIC)

install(TARGETS tasks-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES tasks-mobile.sh DESTINATION ${BIN_INSTALL_DIR})
install(FILES tasks-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  tasks.qml
  tasks-mobile-background.png
  TaskActions.qml
  TaskListView.qml
  DESTINATION ${DATA_INSTALL_DIR}/tasks-mobile
)
kde4_install_icons( ${ICON_INSTALL_DIR} )
