include_directories(${CMAKE_SOURCE_DIR}/akonadi/)

set(mobileui_SRCS
    listproxy.cpp
    breadcrumbnavigation.cpp
    kbreadcrumbnavigationcomponentfactory.cpp
    akonadibreadcrumbnavigationfactory.cpp
    declarativeakonadiitem.cpp
    kdeclarativeapplication.cpp
    kdeclarativemainview_p.cpp
    kdeclarativemainview.cpp
    kdeclarativefullscreenview.cpp
)


kde4_add_library(kdepimmobileui SHARED ${mobileui_SRCS})
set_target_properties(kdepimmobileui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_VERSION})

target_link_libraries(kdepimmobileui
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDEPIMLIBS_AKONADI_LIBRARY}
  akonadi_next
)

install(TARGETS kdepimmobileui ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  qmldir
  Action.qml
  AttachmentList.qml
  Button.qml
  ItemListView.qml
  ItemListViewDelegate.qml
  MainView.qml
  StartCanvas.qml
  WindowBar.qml
  DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui)
