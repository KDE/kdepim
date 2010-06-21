include_directories(
  ${CMAKE_SOURCE_DIR}/akonadi/
  ${KDEPIMLIBS_INCLUDE_DIR}
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)


#include_directories(
#
#  ${KDE4_INCLUDES}
#
#  ${QT_INCLUDES}
#  ${Boost_INCLUDE_DIR}
#  ${CMAKE_CURRENT_SOURCE_DIR}
#)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(mobile_tasks_SRCS
  tasklistproxy.cpp
  mainview.cpp
  main.cpp
)

kde4_add_executable(tasks-mobile ${mobile_tasks_SRCS} )
target_link_libraries(tasks-mobile
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
  kdepimmobileui
#    ${KDEPIMLIBS_AKONADI_KMIME_LIBRARY}
#    ${KDEPIMLIBS_AKONADI_LIBRARY}
#    akonadi_next
    kdepim
)

install(TARGETS tasks-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES tasks-mobile.sh DESTINATION ${BIN_INSTALL_DIR})
install(FILES tasks-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  tasks.qml
  back-to-message-list.png
  DESTINATION ${DATA_INSTALL_DIR}/tasks-mobile
)
kde4_install_icons( ${ICON_INSTALL_DIR} )
