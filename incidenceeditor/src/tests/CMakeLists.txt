
if (BUILD_TESTING)
  set(testincidenceeditor_SRCS main.cpp)

  add_executable(kincidenceeditor ${testincidenceeditor_SRCS})

  target_link_libraries(kincidenceeditor
    KF5::AkonadiCalendar
    KF5::AkonadiCore
    KF5::CalendarCore
    KF5::CalendarUtils
    KF5::IncidenceEditorsng
    KF5::CalendarSupport
    KF5::KdepimDBusInterfaces
  )

  install(TARGETS kincidenceeditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
endif()
