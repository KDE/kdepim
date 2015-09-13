project(coisceim)



include_directories( ${CMAKE_SOURCE_DIR}/messageviewer )

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

add_library(coisceim_widget ${coisceim_widget_SRCS})
generate_export_header(coisceim_widget BASE_NAME coisceim_widget)

qt5_generate_dbus_interface(coisceimwidget.h
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
  KF5::MessageViewer KF5::CalendarSupport
)

set_target_properties(coisceim_widget
    PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION}
)

add_executable(coisceim ${coisceim_SRCS})

target_link_libraries(coisceim
  coisceim_widget
)

add_library(coisceimpart MODULE ${coisceim_part_SRCS})

target_link_libraries(coisceimpart
  coisceim_widget
  
)

install(TARGETS coisceim ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS coisceim_widget ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
install(FILES coisceimpart.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )
install(TARGETS coisceimpart  DESTINATION ${KDE_INSTALL_PLUGINDIR} )

# add_subdirectory(kontact-plugin)

