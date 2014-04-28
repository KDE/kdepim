include_directories(
    ${CMAKE_SOURCE_DIR}/incidenceeditor-ng
    ${CMAKE_SOURCE_DIR}
    ${Boost_INCLUDE_DIR}
)

add_subdirectory( calendar )
add_subdirectory( docs )

set(mobileui_SRCS
    agentstatusmonitor.cpp
    akonadibreadcrumbnavigationfactory.cpp
    breadcrumbnavigation.cpp
    breadcrumbnavigationcontext.cpp
    collectionfetchwatcher.cpp
    declarativewidgetbase.cpp
    declarativeakonadiitem.cpp
    exporthandlerbase.cpp
    favoriteseditor.cpp
    favoritescontroller.cpp
    favoriteslistmodel.cpp
    guistatemanager.cpp
    hierarchyresolver.cpp
    importhandlerbase.cpp
    kcolorcombo.cpp
    kdeclarativeapplication.cpp
    kdeclarativefullscreenview.cpp
    kdeclarativemainview.cpp
    kdeclarativemainview_p.cpp
    kresettingproxymodel.cpp
    listproxy.cpp
    modelselectiontransition.cpp
    orderedchildcollectionsmodel.cpp
    qmllistselectionmodel.cpp
    rangemanager.cpp
    searchmanager.cpp
    statemachinebuilder.cpp
    stylesheetloader.cpp
    threadmodel.cpp
    threadgroupermodel.cpp
    threadselectionmodel.cpp
)

if (KDEQMLPLUGIN_STATIC)
  add_definitions(-DKDEQMLPLUGIN_STATIC)
endif ()

kde_enable_exceptions()

add_library(kdepimmobileui ${LIBRARY_TYPE} ${mobileui_SRCS})
set_target_properties(kdepimmobileui PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})

target_link_libraries(kdepimmobileui
  Qt5::Declarative
  ${QT_QTOPENGL_LIBRARY}
  Qt5::Declarative
  KF5::AkonadiMime
  ${KDE4_SOLID_LIBRARY}
  akonadi_next
  kdepim
  calendarsupport
)

if (KDEQMLPLUGIN_STATIC)
  target_link_libraries(kdepimmobileui kdeqmlplugin)
endif ()

if (X11_FOUND)
  target_link_libraries(kdepimmobileui ${X11_LIBRARIES})
endif()

set( STYLE_IMAGE_PATH ${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui/images )

configure_file( stylesheet.css.cmake ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css )

add_subdirectory(images)

install(TARGETS kdepimmobileui ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  qmldir
  Action.qml
  ActionButton.qml
  ActiveActionMenuItemDelegate.qml
  BulkActionScreen.qml
  BulkActionList.qml
  DecoratedListView.qml
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
  AgentInstanceListDelegate.qml
  AgentInstanceStatusIndicator.qml
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
  VerticalSelector.qml
  Clock.qml
  Calendar.qml
  CalendarDay.qml
  AgentStatusIndicator.qml
  SearchDialog.qml
  SearchResultScreen.qml
  ItemEditButton.qml
  Switch.qml
  DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/pim/mobileui)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css
  ScreenFunctions.js
  DESTINATION ${DATA_INSTALL_DIR}/mobileui)

add_subdirectory( tests )
