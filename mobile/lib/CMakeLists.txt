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

set( STYLE_IMAGE_PATH ${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui )
configure_file( stylesheet.css.cmake ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css ) 

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
  WindowBar.qml
  sliderbackground.png
  sliderhandle.png
  button-left.png
  button-right.png
  button-center.png
  button-border.png
  button-border-active.png
  button-plus.png
  button-plus-active.png
  button-plus-disabled.png
  button-minus.png
  button-minus-active.png
  button-minus-disabled.png
  DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css
  DESTINATION ${DATA_INSTALL_DIR}/mobileui)
