
project(coisceim-mobile)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_SOURCE_DIR}/mobile/lib
                   )

set(coisceim_mobile_srcs
  main.cpp
  mobile_mainview.cpp
  messageviewitem.cpp
)

add_executable(coisceim-mobile ${coisceim_mobile_srcs})

target_link_libraries(coisceim-mobile
  Qt5::Declarative
  KF5::CalendarCore
  ${KDEPIMLIBS_AKONADI_CALENDAR_LIBS}
  calendarsupport

  kdepimmobileuicalendar

  kdepimdbusinterfaces
  kdepimmobileui
  kdepimmobileuicalendar
  kdepim
  eventviews
  coisceim_widget
  messageviewer
)

install(TARGETS coisceim-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
# install(PROGRAMS coisceim-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  main.qml
  CreateTrip.qml
  Trip.qml
  TypeSelection.qml
  NotesListView.qml
  TaskListView.qml
  HeaderView.qml
  ItemViewer.qml
  NoteView.qml
  notes-mobile-background.png
  DESTINATION ${DATA_INSTALL_DIR}/coisceim-mobile
)
