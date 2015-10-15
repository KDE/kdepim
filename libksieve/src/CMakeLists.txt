
include(CheckTypeSize)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)
configure_file(config-libksieve.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-libksieve.h)

set(libksieve_parser_SRCS
  parser/utf8validator.cpp
  parser/lexer.cpp
  parser/parser.cpp
)

add_definitions(-DTRANSLATION_DOMAIN=\"libksieve\")

add_subdirectory(kmanagesieve)
add_subdirectory(ksieveui)


########### next target ###############

set(ksieve_LIB_SRCS
  shared/error.cpp
  ${libksieve_parser_SRCS}
)

add_library(KF5KSieve ${ksieve_LIB_SRCS})
generate_export_header(KF5KSieve BASE_NAME ksieve)
add_library(KF5::KSieve ALIAS KF5KSieve)

target_link_libraries(KF5KSieve
PRIVATE
  Qt5::Core KF5::I18n
)

set_target_properties(KF5KSieve PROPERTIES
  VERSION ${LIBKSIEVE_VERSION_STRING}
  SOVERSION ${LIBKSIEVE_SOVERSION}
  EXPORT_NAME KSieve
)

#target_include_directories(KF5KSieve INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KSieve/;${KDE_INSTALL_INCLUDEDIR_KF5}/ksieve>")
target_include_directories(KF5KSieve PUBLIC "$<BUILD_INTERFACE:${libksieve_SOURCE_DIR}/src;${libksieve_BINARY_DIR}/src>")
target_include_directories(KF5KSieve PUBLIC "$<BUILD_INTERFACE:${libksieve_SOURCE_DIR}/src/parser>")


ecm_generate_pri_file(BASE_NAME KSieve
    LIB_NAME KF5KSieve
    DEPS "" FILENAME_VAR PRI_FILENAME
)


install(TARGETS KF5KSieve EXPORT KF5KSieveTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} ${LIBRARY_NAMELINK})
if(${CMAKE_SOURCE_DIR} STREQUAL ${libksieve_SOURCE_DIR})
install(FILES
    ${ksieve_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KSieve
    COMPONENT Devel
)

install(FILES
    ${ksieve_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/ksieve
    COMPONENT Devel
)
install(FILES
    ${PRI_FILENAME}
    DESTINATION ${ECM_MKSPECS_INSTALL_DIR}
)

endif()
