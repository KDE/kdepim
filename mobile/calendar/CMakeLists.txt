project(korganizer-mobile)

include_directories(
  # This is probably not the correct way to get KCalPrefs in.
  ${CMAKE_SOURCE_DIR}/calendarsupport/
  ${CMAKE_BINARY_DIR}/calendarsupport/
  ${CMAKE_SOURCE_DIR}/calendarviews/eventviews
  ${KDEPIMLIBS_INCLUDE_DIR}
  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(korganizer_mobile_SRCS
  agendaviewitem.cpp
  monthviewitem.cpp
  calendarinterface.cpp
  declarativewidgetwrapper.cpp
  eventlistproxy.cpp
  qmldateedit.cpp
  mainview.cpp
  main.cpp
)

qt4_add_dbus_adaptor(korganizer_mobile_SRCS ${CMAKE_SOURCE_DIR}/korganizer/org.kde.Korganizer.Calendar.xml calendarinterface.h CalendarInterface )

kde4_add_executable(korganizer-mobile ${korganizer_mobile_SRCS} )
if (WINCE)
  SET_TARGET_PROPERTIES(korganizer-mobile PROPERTIES LINK_FLAGS " /FIXED:NO")
endif (WINCE)
  
target_link_libraries(korganizer-mobile
  ${QT_QTDECLARATIVE_LIBRARY}
  ${KDE4_KDECORE_LIBRARY}
  ${KDE4_KDEUI_LIBRARY}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  kdepimdbusinterfaces
  kdepimmobileui
  kdepimmobileuicalendar
  calendarsupport
  incidenceeditorsngmobile
  eventviews
  calendarsupport
  kdepim
)

install(TARGETS korganizer-mobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES korganizer-mobile.sh DESTINATION ${BIN_INSTALL_DIR})
install(FILES korganizer-mobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES
  korganizer-mobile.qml
  KorganizerActions.qml
  korganizer-mobile-background.png
  EventListView.qml
  DESTINATION ${DATA_INSTALL_DIR}/korganizer-mobile
)
kde4_install_icons( ${ICON_INSTALL_DIR} )
