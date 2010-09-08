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

kde4_add_executable(tasks-mobile ${mobile_tasks_SRCS} )
target_link_libraries(tasks-mobile
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  kdepimmobileui
  kdepimmobileuicalendar
  kdepim
)

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
