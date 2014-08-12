project(calendarjanitor)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5860)

########### next target ###############

set(calendarjanitor_SRCS
    backuper.cpp
    calendarjanitor.cpp
    collectionloader.cpp
    main.cpp
    options.cpp)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_BINARY_DIR}/calendarsupport


  ${AKONADI_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${QT_INCLUDES}
  ${ZLIB_INCLUDE_DIRS}
)
   

kde4_add_app_icon(calendarjanitor_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/office-calendar.png")

add_executable(calendarjanitor ${calendarjanitor_SRCS})

target_link_libraries(calendarjanitor
                      
                      KF5::CalendarUtils
                      KF5::CalendarCore
                      kdepim
                      calendarsupport
                      KF5::AkonadiCalendar)

install(TARGETS calendarjanitor  ${INSTALL_TARGETS_DEFAULT_ARGS})
