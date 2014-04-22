project(akonadi_next)

if(CMAKE_SOURCE_DIR STREQUAL "${akonadi_next_SOURCE_DIR}")
  find_package(KDE4 REQUIRED)
  include(KDE4Defaults)
  find_package(Akonadi REQUIRED)
  find_package(KdepimLibs REQUIRED)
endif()

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


kde_enable_exceptions()

set(akonadi_next_SRCS
  note.cpp
  notecreatorandselector.cpp
  quotacolorproxymodel.cpp
  kcolumnfilterproxymodel.cpp
  amazingcompleter.cpp
  amazingdelegate.cpp
)

add_library(akonadi_next ${LIBRARY_TYPE} ${akonadi_next_SRCS})

target_link_libraries(akonadi_next
  ${AKONADI_COMMON_LIBRARIES}
  KF5::AkonadiCore
  KF5::KIOCore
  KF5::KMime
)

set_target_properties(akonadi_next PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS akonadi_next ${INSTALL_TARGETS_DEFAULT_ARGS})

