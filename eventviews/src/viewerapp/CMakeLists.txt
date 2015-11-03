
set(viewerapp_SRCS
  main.cpp
  mainwindow.cpp
  ../calendarview_debug.cpp
)

kconfig_add_kcfg_files(viewerapp_SRCS settings.kcfgc)

ki18n_wrap_ui(viewerapp_SRCS
  mainwindow.ui
)

add_executable(viewerapp ${viewerapp_SRCS})

target_link_libraries(viewerapp
  KF5::CalendarSupport
  KF5::EventViews
  KF5::AkonadiWidgets
  KF5::CalendarCore
  KF5::AkonadiCalendar
)

########### install files ###############

#install(TARGETS viewerapp ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
#install(PROGRAMS viewerapp.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
