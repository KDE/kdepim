include_directories(
    ${CMAKE_SOURCE_DIR}/incidenceeditor-ng
    ${CMAKE_SOURCE_DIR}/calendarsupport
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${Boost_INCLUDE_DIR}
)

set(mobileui_calendar_SRCS
    calendarhelper.cpp
    clockhelper.cpp
    declarativeeditors.cpp
    kcalitembrowseritem.cpp
    incidenceview.cpp
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_ui_files(mobileui_calendar_SRCS
  ${CMAKE_SOURCE_DIR}/incidenceeditor-ng/eventortodomobile.ui
  ${CMAKE_SOURCE_DIR}/incidenceeditor-ng/eventortodomoremobile.ui
)

kde4_add_library(kdepimmobileuicalendar STATIC ${mobileui_calendar_SRCS})

target_link_libraries(kdepimmobileuicalendar
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  akonadi_next
  incidenceeditorsngmobile
  calendarsupport
)

install(FILES
  incidence-editor.qml
  DESTINATION ${DATA_INSTALL_DIR}/mobileui)
