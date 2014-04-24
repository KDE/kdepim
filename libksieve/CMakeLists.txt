project(libksieve)

include(CheckTypeSize)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)
configure_file(config-libksieve.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-libksieve.h)

set(libksieve_parser_SRCS
  ${CMAKE_SOURCE_DIR}/libksieve/parser/utf8validator.cpp
  ${CMAKE_SOURCE_DIR}/libksieve/parser/lexer.cpp
  ${CMAKE_SOURCE_DIR}/libksieve/parser/parser.cpp
)

add_subdirectory(ksieve)
add_subdirectory(kmanagesieve)
add_subdirectory(ksieveui)
add_subdirectory(tests)

########### next target ###############

set(ksieve_LIB_SRCS
  ${CMAKE_SOURCE_DIR}/libksieve/shared/error.cpp
  ${libksieve_parser_SRCS}
)

add_library(ksieve ${LIBRARY_TYPE} ${ksieve_LIB_SRCS})

target_link_libraries(ksieve Qt5::Core KF5::KDELibs4Support)

set_target_properties(
  ksieve PROPERTIES
  VERSION ${GENERIC_LIB_VERSION}
  SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS ksieve ${INSTALL_TARGETS_DEFAULT_ARGS})

