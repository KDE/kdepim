project(kaddressbookgrantlee)
kde_enable_exceptions()

include_directories(
  ${Grantlee_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/grantleetheme 
)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions(-DTRANSLATION_DOMAIN=\"libkaddressbookgrantlee\")

if(Prison_FOUND)
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
target_link_libraries(kaddressbookgrantlee
  Grantlee::Templates
  grantleetheme
  KF5::AkonadiCore
  KF5::Abc
  KF5::AkonadiContact
)

set_target_properties(kaddressbookgrantlee PROPERTIES VERSION ${KDEPIM_LIB_VERSION} SOVERSION ${KDEPIM_LIB_SOVERSION})
install(TARGETS kaddressbookgrantlee ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

