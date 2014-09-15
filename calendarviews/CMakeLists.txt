# The following macros can be defined to alter behavior.
# (if desired, use add_definitions() to define them)
#
# EVENTVIEWS_NODECOS - turns-off decoration plugins in views.
#                No idea why you would want to define this, perhaps to save
#                screen real estate? But there are a config options for that.

project(calendarView)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


if(KDEPIM_BUILD_EXAMPLES)
  add_subdirectory(viewerapp)
endif()

option(EVENTVIEWS_NODECOS "Turn-off decoration plugins in views." FALSE)

if(EVENTVIEWS_NODECOS)
  add_definitions(-DEVENTVIEWS_NODECOS)
endif()

include_directories(
  ${Boost_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/akonadi_next
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_BINARY_DIR}/calendarsupport
  ${CMAKE_SOURCE_DIR}/calendarviews
  ${CMAKE_SOURCE_DIR}/libkdepim/
  ${ZLIB_INCLUDE_DIRS}
)

if(KDEPIM_MOBILE_UI)
  add_definitions(-DKORG_NODND)
endif()

########### next target ###############

set(eventviews_LIB_SRCS
  eventview_p.cpp
  eventview.cpp
  helper.cpp
  prefs.cpp

  # Agenda view specific code.
  agenda/agenda.cpp
  agenda/agendaitem.cpp
  agenda/agendaview.cpp
  agenda/alternatelabel.cpp
  agenda/calendardecoration.cpp
  agenda/decorationlabel.cpp
  agenda/timelabels.cpp
  agenda/timelabelszone.cpp
  agenda/timescaleconfigdialog.cpp
  agenda/viewcalendar.cpp

  journal/journalframe.cpp
  journal/journalview.cpp

  list/listview.cpp

  month/monthgraphicsitems.cpp
  month/monthitem.cpp
  month/monthscene.cpp
  month/monthview.cpp

  multiagenda/multiagendaview.cpp

  todo/tododelegates.cpp
  todo/todomodel.cpp
  todo/incidencetreemodel.cpp

  todo/todoviewquickaddline.cpp
  todo/todoviewquicksearch.cpp
  todo/todoviewsortfilterproxymodel.cpp
  todo/todoviewview.cpp
  todo/todoview.cpp

  timeline/timelineview.cpp
  timeline/timelineitem.cpp
  timeline/timelineview_p.cpp

  timespent/timespentview.cpp

  whatsnext/whatsnextview.cpp
)

kde4_add_kcfg_files(eventviews_LIB_SRCS prefs_base.kcfgc)

kde4_add_ui_files(eventviews_LIB_SRCS
  agenda/timescaleedit_base.ui
)

kde4_add_library(eventviews ${LIBRARY_TYPE} ${eventviews_LIB_SRCS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

target_link_libraries(eventviews
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTUITOOLS_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDE4_KCMUTILS_LIBS}
  ${ZLIB_LIBRARIES}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KHOLIDAYS_LIBS}
  kdepim
  kdgantt2
  calendarsupport
  akonadi_next
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDEPIMLIBS_KCALUTILS_LIBS}
  akonadi-calendar
)

set_target_properties(eventviews PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS eventviews ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES agenda/calendardecoration.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
