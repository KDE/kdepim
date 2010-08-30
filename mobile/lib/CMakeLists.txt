include_directories(
    ${CMAKE_SOURCE_DIR}/incidenceeditors/incidenceeditor-ng
    ${Boost_INCLUDE_DIR}
)

add_subdirectory( calendar )

set(mobileui_SRCS
    listproxy.cpp
    breadcrumbnavigation.cpp
    breadcrumbnavigationcontext.cpp
    akonadibreadcrumbnavigationfactory.cpp
    favoriteslistmodel.cpp
    kresettingproxymodel.cpp
    declarativeakonadiitem.cpp
    kdeclarativeapplication.cpp
    kdeclarativemainview_p.cpp
    kdeclarativemainview.cpp
    kdeclarativefullscreenview.cpp
    stylesheetloader.cpp
    qmllistselectionmodel.cpp
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_library(kdepimmobileui ${LIBRARY_TYPE} ${mobileui_SRCS})
set_target_properties(kdepimmobileui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(kdepimmobileui
  ${QT_QTDECLARATIVE_LIBRARY}
  ${QT_QTOPENGL_LIBRARY}
  ${KDEPIMLIBS_AKONADI_LIBS}
  akonadi_next
  kdepim
  calendarsupport
)

set( STYLE_IMAGE_PATH ${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui/images )
configure_file( stylesheet.css.cmake ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css )

add_subdirectory(images)

install(TARGETS kdepimmobileui ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  qmldir
  Action.qml
  ActiveActionMenuItemDelegate.qml
  BulkActionScreen.qml
  BulkActionList.qml
  FakeAction.qml
  FavoriteManager.qml
  ActionList.qml
  ReorderList.qml
  ReorderListDelegate.qml
  MultipleSelectionComponent.qml
  MultipleSelectionScreen.qml
  ReorderListContainer.qml
  ActionListItem.qml
  ActionMenuContainer.qml
  AgentInstanceList.qml
  AgentInstanceListContainer.qml
  AttachmentList.qml
  ApplicationGeneralActions.qml
  Button.qml
  Button2.qml
  AboutDialog.qml
  CompletionSlider.qml
  ItemListView.qml
  ItemListViewDelegate.qml
  MainView.qml
  ScriptAction.qml
  ScriptActionItem.qml
  StartCanvas.qml
  Spinner.qml
  WindowBar.qml
  VerticalFadeSelector.qml
  Clock.qml
  DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css
  DESTINATION ${DATA_INSTALL_DIR}/mobileui)
