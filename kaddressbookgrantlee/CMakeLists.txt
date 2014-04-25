project(kaddressbookgrantlee)

include_directories(
  ${Grantlee_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/grantleetheme 
)

add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )


kde_enable_exceptions()

if(Prison_FOUND)
  add_definitions(-DHAVE_PRISON)
  include_directories(${PRISON_INCLUDE_DIR})
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

if(PRISON_FOUND)
  target_link_libraries(kaddressbookgrantlee ${PRISON_LIBRARIES})
endif()

set_target_properties(kaddressbookgrantlee PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kaddressbookgrantlee ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

