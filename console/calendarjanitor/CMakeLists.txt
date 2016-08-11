project(calendarjanitor)

########### next target ###############

set(calendarjanitor_SRCS
    backuper.cpp
    calendarjanitor.cpp
    collectionloader.cpp
    main.cpp
    options.cpp)

add_executable(calendarjanitor ${calendarjanitor_SRCS})

target_link_libraries(calendarjanitor
    KF5::CalendarUtils
    KF5::CalendarCore
    KF5::Libkdepim
    KF5::CalendarSupport
    KF5::AkonadiCalendar)

install(TARGETS calendarjanitor  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
