include(ECMAddTests)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DMAIL_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\"")

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/templateparser/
  ${CMAKE_BINARY_DIR}/templateparser/
)
#include ( ${CMAKE_SOURCE_DIR}/cmake/modules/kde4_handle_crypto_rpath_for_executable.cmake )

# convenience macro to add qtest unit tests
macro(add_templateparser_unittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(templateparser-${_name})
  target_link_libraries(${_name}
    kleo
    templateparser
    messageviewer
    KF5::QGpgme
    Qt5::Test
    Qt5::WebKitWidgets
    KF5::KIOCore
    KF5::Mime
    KF5::AkonadiCore
    KF5::IdentityManagement
  )
endmacro()

add_templateparser_unittest(templateparsertest.cpp)
#kde4_handle_crypto_rpath_for_executable( templateparsertest )

