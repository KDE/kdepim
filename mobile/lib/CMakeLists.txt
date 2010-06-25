include_directories(
    ${CMAKE_SOURCE_DIR}/akonadi/
    ${Boost_INCLUDE_DIR}
)

set(mobileui_SRCS
    listproxy.cpp
    breadcrumbnavigation.cpp
    kbreadcrumbnavigationcomponentfactory.cpp
    akonadibreadcrumbnavigationfactory.cpp
    kresettingproxymodel.cpp
    declarativeakonadiitem.cpp
    kdeclarativeapplication.cpp
    kdeclarativemainview_p.cpp
    kdeclarativemainview.cpp
    kdeclarativefullscreenview.cpp
    stylesheetloader.cpp
)


kde4_add_library(kdepimmobileui SHARED ${mobileui_SRCS})
set_target_properties(kdepimmobileui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_VERSION})

target_link_libraries(kdepimmobileui
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDEPIMLIBS_AKONADI_LIBRARY}
  akonadi_next
  messagecore
)

set( STYLE_IMAGE_PATH ${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui/images )
configure_file( stylesheet.css.cmake ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css )

add_subdirectory(images)

install(TARGETS kdepimmobileui ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  qmldir
  Action.qml
  AttachmentList.qml
  Button.qml
  Button2.qml
  CompletionSlider.qml
  ItemListView.qml
  ItemListViewDelegate.qml
  MainView.qml
  StartCanvas.qml
  Spinner.qml
  WindowBar.qml
  DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css
  DESTINATION ${DATA_INSTALL_DIR}/mobileui)
