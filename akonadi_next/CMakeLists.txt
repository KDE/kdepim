project(akonadi_next)


add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_next\")

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
  KF5::Mime
  KF5::Abc
)

set_target_properties(akonadi_next PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})

install(TARGETS akonadi_next ${INSTALL_TARGETS_DEFAULT_ARGS})

