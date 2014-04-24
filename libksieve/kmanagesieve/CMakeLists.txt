find_package(Sasl2)
set_package_properties(Sasl2 PROPERTIES DESCRIPTION "Cyrus SASL API" URL "http://asg.web.cmu.edu/sasl/sasl-library.html" TYPE REQUIRED PURPOSE "Required to support authentication of logins on ManageSieve servers.")

include_directories(${SASL2_INCLUDE_DIR})

set(kmanagesieve_LIB_SRCS
  response.cpp
  session.cpp
  sievejob.cpp
)

add_library(kmanagesieve ${LIBRARY_TYPE} ${kmanagesieve_LIB_SRCS})

target_link_libraries(kmanagesieve
  KF5::KDELibs4Support  
  KF5::KIOCore
  ${SASL2_LIBRARIES}
)

set_target_properties(
  kmanagesieve PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS kmanagesieve ${INSTALL_TARGETS_DEFAULT_ARGS})
