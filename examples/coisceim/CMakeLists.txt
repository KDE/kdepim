project(coisceim)

kde_enable_exceptions()

include_directories( ${Boost_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/messageviewer )

set (coisceim_widget_SRCS
  coisceimwidget.cpp
  note.cpp
  foldercontentswidget.cpp
  tripwidget.cpp
  createtripwidget.cpp
  createfoldercontentswidget.cpp
  tripmodel.cpp
  stackedwidgetview.cpp
  eventselectorwidget.cpp
  trip.cpp
  tripcomponentfactory.cpp
  mixedtreemodel.cpp
  itemselection.cpp
  itemviewerwidget.cpp
  noteviewer.cpp
  todocheckableproxymodel.cpp
  qmllistselectionmodel.cpp
)

set (coisceim_SRCS
  main.cpp
)

set (coisceim_part_SRCS
  coisceimpart.cpp
)

add_library(coisceim_widget ${LIBRARY_TYPE} ${coisceim_widget_SRCS})

qt4_generate_dbus_interface(coisceimwidget.h
  org.kde.coisceim.CoisceimWidget.xml
)

qt5_add_dbus_adaptor(coisceim_widget_SRCS
  ${CMAKE_BINARY_DIR}/org.kde.coisceim.CoisceimWidget.xml
  coisceimwidget.h
  CoisceimWidget
)

target_link_libraries(coisceim_widget
  KF5::AkonadiCore
  KF5::AkonadiContact
  KF5::CalendarCore
  messageviewer calendarsupport
)

set_target_properties(coisceim_widget
    PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION}
)

add_executable(coisceim ${coisceim_SRCS})

target_link_libraries(coisceim
  coisceim_widget
  ${QT_QTCORE_LIBRARIES}
  
  
)

add_library(coisceimpart MODULE ${coisceim_part_SRCS})

target_link_libraries(coisceimpart
  coisceim_widget
  
)

install(TARGETS coisceim ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS coisceim_widget ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
install(FILES coisceimpart.desktop  DESTINATION ${SERVICES_INSTALL_DIR} )
install(TARGETS coisceimpart  DESTINATION ${PLUGIN_INSTALL_DIR} )

# add_subdirectory(kontact-plugin)
add_subdirectory(coisceim-mobile)

