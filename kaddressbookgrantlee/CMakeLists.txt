project(kaddressbookgrantlee)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions(-DTRANSLATION_DOMAIN=\"libkaddressbookgrantlee\")

if(KF5Prison_FOUND)
  add_definitions(-DHAVE_PRISON)
endif()

set(kaddressbook_grantlee_LIB_SRCS
  widget/grantleecontactviewer.cpp
  formatter/grantleecontactformatter.cpp
  formatter/grantleecontactgroupformatter.cpp
  formatter/grantleecontactutils.cpp
)

set(kaddressbook_grantlee_printing_LIB_SRCS
   printing/contactgrantleeprintaddressobject.cpp
   printing/contactgrantleeprintcryptoobject.cpp
   printing/contactgrantleeprintgeoobject.cpp
   printing/contactgrantleeprintimobject.cpp
   printing/contactgrantleeprintobject.cpp
   printing/contactgrantleeprintphoneobject.cpp
   printing/grantleeprint.cpp
)


add_library(kaddressbookgrantlee ${kaddressbook_grantlee_LIB_SRCS} ${kaddressbook_grantlee_printing_LIB_SRCS})
generate_export_header(kaddressbookgrantlee BASE_NAME kaddressbook_grantlee)
target_link_libraries(kaddressbookgrantlee
PUBLIC
  Grantlee5::Templates
PRIVATE
  KF5::ConfigWidgets
  KF5::Service
  KF5::GrantleeTheme
  KF5::AkonadiCore
  KF5::Contacts
  KF5::AkonadiContact
  KF5::IconThemes
  kleo
  KF5::I18n
)

target_include_directories(kaddressbookgrantlee PUBLIC "$<BUILD_INTERFACE:${kaddressbookgrantlee_SOURCE_DIR};${kaddressbookgrantlee_BINARY_DIR};${Grantlee_INCLUDE_DIRS}>")

set_target_properties(kaddressbookgrantlee PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS kaddressbookgrantlee ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)


if (BUILD_TESTING)
   add_subdirectory(printing/autotests)
endif()

