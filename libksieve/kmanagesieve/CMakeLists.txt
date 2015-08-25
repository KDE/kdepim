find_package(Sasl2)
set_package_properties(Sasl2 PROPERTIES DESCRIPTION "Cyrus SASL API" URL "http://asg.web.cmu.edu/sasl/sasl-library.html" TYPE REQUIRED PURPOSE "Required to support authentication of logins on ManageSieve servers.")

include_directories(${SASL2_INCLUDE_DIR})

set(kmanagesieve_LIB_SRCS
  response.cpp
  session.cpp
  sievejob.cpp
)
ecm_qt_declare_logging_category(kmanagesieve_LIB_SRCS HEADER kmanagersieve_debug.h IDENTIFIER KMANAGERSIEVE_LOG CATEGORY_NAME log_kmanagersieve)

add_library(kmanagesieve ${kmanagesieve_LIB_SRCS})
generate_export_header(kmanagesieve BASE_NAME kmanagesieve)

target_link_libraries(kmanagesieve
PRIVATE
  KF5::I18n 
  KF5::KIOCore
  KF5::KIOWidgets
  ${Sasl2_LIBRARIES}
)

target_include_directories(kmanagesieve PUBLIC "$<BUILD_INTERFACE:${libksieve_SOURCE_DIR}/kmanagesieve;${libksieve_BINARY_DIR}/kmanagesieve>")

set_target_properties(
  kmanagesieve PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS kmanagesieve ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
