# The following macros can be defined to alter behavior.
# (if desired, use add_definitions() to define them)
#
# EVENTVIEWS_NODECOS - turns-off decoration plugins in views.
#                No idea why you would want to define this, perhaps to save
#                screen real estate? But there are a config options for that.

project(calendarView)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_definitions(-DTRANSLATION_DOMAIN=\"libeventviews\")

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

kconfig_add_kcfg_files(eventviews_LIB_SRCS prefs_base.kcfgc)

ki18n_wrap_ui(eventviews_LIB_SRCS
  agenda/timescaleedit_base.ui
)

add_library(eventviews ${LIBRARY_TYPE} ${eventviews_LIB_SRCS})

target_link_libraries(eventviews
  KF5::Mime
  KF5::KCMUtils
  ${ZLIB_LIBRARIES}
  KF5::PimIdentities
  KF5::KIOCore
  KF5::AkonadiCore
  KF5::Holidays
  kdepim
  kdgantt2
  calendarsupport
  akonadi_next
  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::AkonadiCalendar
)

set_target_properties(eventviews PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS eventviews ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES agenda/calendardecoration.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
