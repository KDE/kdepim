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

kde4_add_kcfg_files(viewerapp_SRCS settings.kcfgc)

kde4_add_ui_files(viewerapp_SRCS
  mainwindow.ui
)

kde4_add_executable(viewerapp ${viewerapp_SRCS})

target_link_libraries(viewerapp
  calendarsupport
  eventviews
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${QT_QTGUI_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  KF5::AkonadiCalendar
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

########### install files ###############

#install(TARGETS viewerapp ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(PROGRAMS viewerapp.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
