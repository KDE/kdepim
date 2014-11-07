project(kaddressbookgrantlee)


include_directories(
  ${Grantlee_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/grantleetheme 
)

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
  grantleeutil.cpp
)

add_library(kaddressbookgrantlee ${LIBRARY_TYPE} ${kaddressbook_grantlee_LIB_SRCS})
generate_export_header(kaddressbookgrantlee BASE_NAME kaddressbook_grantlee)
target_link_libraries(kaddressbookgrantlee
  Grantlee5::Templates
  grantleetheme
  KF5::AkonadiCore
  KF5::Contacts
  KF5::AkonadiContact
)

set_target_properties(kaddressbookgrantlee PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS kaddressbookgrantlee ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

