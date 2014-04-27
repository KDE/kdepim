set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

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

kde4_add_library(pimprint_calendar ${LIBRARY_TYPE} ${pimprint_calendar_LIB_SRCS})

target_link_libraries(pimprint_calendar
  calendarsupport
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KCALCORE_LIBS}
  ${KDE4_KDECORE_LIBS}
  ${QT_QTGUI_LIBRARY}
)

set_target_properties(pimprint_calendar PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS pimprint_calendar EXPORT kdepimlibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
