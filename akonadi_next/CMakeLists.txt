project(akonadi_next)


add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_next\")

set(akonadi_next_SRCS
  akonadinext_debug.cpp
  quotacolorproxymodel.cpp
  kcolumnfilterproxymodel.cpp
)

add_library(akonadi_next ${akonadi_next_SRCS})
generate_export_header(akonadi_next BASE_NAME akonadi_next)

target_link_libraries(akonadi_next
  KF5::AkonadiPrivate
  KF5::AkonadiCore
  KF5::KIOCore
  KF5::Mime
  KF5::Contacts
  KF5::AkonadiNotes
)

set_target_properties(akonadi_next PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})

install(TARGETS akonadi_next ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

