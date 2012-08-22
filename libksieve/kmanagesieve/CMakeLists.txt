find_package(Sasl2)
macro_log_feature(SASL2_FOUND "cyrus-sasl" "Cyrus SASL API" "http://asg.web.cmu.edu/sasl/sasl-library.html" TRUE "" "Required to support authentication of logins on ManageSieve servers.")

include_directories(${SASL2_INCLUDE_DIR})

set(kmanagesieve_LIB_SRCS
  response.cpp
  session.cpp
  sievejob.cpp
)

kde4_add_library(kmanagesieve ${LIBRARY_TYPE} ${kmanagesieve_LIB_SRCS})

target_link_libraries(kmanagesieve
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KIO_LIBS}
  ${SASL2_LIBRARIES}
)

set_target_properties(
  kmanagesieve PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS kmanagesieve ${INSTALL_TARGETS_DEFAULT_ARGS})
