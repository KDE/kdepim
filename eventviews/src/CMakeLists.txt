# The following macros can be defined to alter behavior.
# (if desired, use add_definitions() to define them)
#
# EVENTVIEWS_NODECOS - turns-off decoration plugins in views.
#                No idea why you would want to define this, perhaps to save
#                screen real estate? But there are a config options for that.

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

kconfig_add_kcfg_files(eventviews_LIB_SRCS prefs_base.kcfgc)

ecm_qt_declare_logging_category(eventviews_LIB_SRCS HEADER calendarview_debug.h IDENTIFIER CALENDARVIEW_LOG CATEGORY_NAME log_calendarview)
ki18n_wrap_ui(eventviews_LIB_SRCS
  agenda/timescaleedit_base.ui
)

add_library(KF5EventViews ${eventviews_LIB_SRCS})
generate_export_header(KF5EventViews BASE_NAME eventviews)

add_library(KF5::EventViews ALIAS KF5EventViews)
target_link_libraries(KF5EventViews
PUBLIC
  KF5::AkonadiCore
  KF5::CalendarSupport

  KF5::CalendarCore
  KF5::CalendarUtils
  KF5::AkonadiCalendar
PRIVATE
  KF5::Libkdepim
  KF5::Codecs
  KF5::KDGantt2
  KF5::AkonadiWidgets
)

target_include_directories(KF5EventViews INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/EventViews/;${KDE_INSTALL_INCLUDEDIR_KF5}/eventviews>")
target_include_directories(KF5EventViews PUBLIC "$<BUILD_INTERFACE:${eventviews_SOURCE_DIR}/src;${eventviews_BINARY_DIR}/src;>")

set_target_properties(KF5EventViews PROPERTIES
    VERSION ${EVENTVIEWS_VERSION_STRING}
    SOVERSION ${EVENTVIEWS_SOVERSION}
    EXPORT_NAME EventViews
)

install(TARGETS
    KF5EventViews
    EXPORT KF5EventViewsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK}
)

ecm_generate_headers(eventviews_CamelCase_HEADERS
  HEADER_NAMES
  Prefs
  EventView
  REQUIRED_HEADERS eventviews_CamelCase_HEADERS
  PREFIX EventViews
)

ecm_generate_headers(eventviews_CamelCaseagenda_HEADERS
  HEADER_NAMES
  CalendarDecoration
  AgendaView
  ViewCalendar
  REQUIRED_HEADERS eventviews_agenda_HEADERS
  PREFIX EventViews
  RELATIVE agenda
)

ecm_generate_headers(eventviews_CamelCasetodo_HEADERS
  HEADER_NAMES
  TodoView
  REQUIRED_HEADERS eventviews_todo_HEADERS
  PREFIX EventViews
  RELATIVE todo
)

ecm_generate_headers(eventviews_CamelCasemonth_HEADERS
  HEADER_NAMES
  MonthView
  REQUIRED_HEADERS eventviews_month_HEADERS
  PREFIX EventViews
  RELATIVE month
)

ecm_generate_headers(eventviews_CamelCaselist_HEADERS
  HEADER_NAMES
  ListView
  REQUIRED_HEADERS eventviews_list_HEADERS
  PREFIX EventViews
  RELATIVE list
)

ecm_generate_headers(eventviews_CamelCasejournal_HEADERS
  HEADER_NAMES
  JournalView
  REQUIRED_HEADERS eventviews_journal_HEADERS
  PREFIX EventViews
  RELATIVE journal
)

ecm_generate_headers(eventviews_CamelCasemultiagenda_HEADERS
  HEADER_NAMES
  MultiAgendaView
  ConfigDialogInterface
  REQUIRED_HEADERS eventviews_multiagenda_HEADERS
  PREFIX EventViews
  RELATIVE multiagenda
)

ecm_generate_headers(eventviews_CamelCasetimeline_HEADERS
  HEADER_NAMES
  TimeLineView
  REQUIRED_HEADERS eventviews_timeline_HEADERS
  PREFIX EventViews
  RELATIVE timeline
)

ecm_generate_headers(eventviews_CamelCasetimespent_HEADERS
  HEADER_NAMES
  TimeSpentView
  REQUIRED_HEADERS eventviews_timespent_HEADERS
  PREFIX EventViews
  RELATIVE timespent
)

ecm_generate_headers(eventviews_CamelCasewhatsnext_HEADERS
  HEADER_NAMES
  WhatsNextView
  REQUIRED_HEADERS eventviews_whatsnext_HEADERS
  PREFIX EventViews
  RELATIVE whatsnext
)
ecm_generate_pri_file(BASE_NAME EventViews
    LIB_NAME KF5EventViews
    DEPS "Mime IdentityManagement AkonadiCalendar" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/EventViews
)


if(${CMAKE_SOURCE_DIR} STREQUAL ${eventviews_SOURCE_DIR})
install(FILES
    ${eventviews_CamelCase_HEADERS}
    ${eventviews_CamelCaseagenda_HEADERS}
    ${eventviews_CamelCasetodo_HEADERS}
    ${eventviews_CamelCasetimespent_HEADERS}
    ${eventviews_CamelCasewhatsnext_HEADERS}
    ${eventviews_CamelCasetimeline_HEADERS}
    ${eventviews_CamelCasemultiagenda_HEADERS}
    ${eventviews_CamelCasejournal_HEADERS}
    ${eventviews_CamelCaselist_HEADERS}
    ${eventviews_CamelCasemonth_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/EventViews
    COMPONENT Devel
)

install(FILES
    ${eventviews_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/eventviews_export.h
    ${eventviews_CamelCase_HEADERS}
    ${eventviews_agenda_HEADERS}
    ${eventviews_todo_HEADERS}
    ${eventviews_timespent_HEADERS}
    ${eventviews_whatsnext_HEADERS}
    ${eventviews_timeline_HEADERS}
    ${eventviews_multiagenda_HEADERS}
    ${eventviews_journal_HEADERS}
    ${eventviews_list_HEADERS}
    ${eventviews_month_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/eventviews
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR})


endif()

install(FILES agenda/calendardecoration.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})


