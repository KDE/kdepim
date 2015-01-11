project(calendarviewsapp)

include_directories(
  ${Boost_INCLUDE_DIR}
  ${calendarView_SOURCE_DIR}/
  ${calendarView_SOURCE_DIR}/agenda
)

set(viewerapp_SRCS
  main.cpp
  mainwindow.cpp
)

kconfig_add_kcfg_files(viewerapp_SRCS settings.kcfgc)

ki18n_wrap_ui(viewerapp_SRCS
  mainwindow.ui
)

add_executable(viewerapp ${viewerapp_SRCS})

target_link_libraries(viewerapp
  calendarsupport
  eventviews
  KF5::AkonadiCore
  KF5::CalendarCore
  KF5::AkonadiCalendar
)

########### install files ###############

#install(TARGETS viewerapp ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
#install(PROGRAMS viewerapp.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
