
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

add_library(ksieve ${ksieve_LIB_SRCS})
generate_export_header(ksieve BASE_NAME ksieve)


target_link_libraries(ksieve 
PRIVATE
  Qt5::Core KF5::I18n
)

target_include_directories(ksieve PUBLIC "$<BUILD_INTERFACE:${libksieve_SOURCE_DIR}/parser>")

set_target_properties(
  ksieve PROPERTIES
  VERSION ${KDEPIM_LIB_VERSION}
  SOVERSION ${KDEPIM_LIB_SOVERSION}
)

install(TARGETS ksieve ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

