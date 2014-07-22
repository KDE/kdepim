kde_enable_exceptions()

include_directories(
  ${CMAKE_SOURCE_DIR}/calendarsupport
  ${CMAKE_BINARY_DIR}/calendarsupport
)

set(pimprint_calendar_LIB_SRCS
  cellitem.cpp
  calprintbase.cpp
  calprintday.cpp
  calprintmonth.cpp
  calprinttodos.cpp
  calprintweek.cpp
)

add_library(pimprint_calendar ${LIBRARY_TYPE} ${pimprint_calendar_LIB_SRCS})
generate_export_header(pimprint_calendar BASE_NAME pimprint_calendar)

target_link_libraries(pimprint_calendar
  calendarsupport
  KF5::AkonadiCore
  KF5::CalendarCore
  
  
)

set_target_properties(pimprint_calendar PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS pimprint_calendar EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
