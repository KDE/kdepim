include(ECMAddTests)

include_directories (
    ${CMAKE_SOURCE_DIR}/libkleo
    ${CMAKE_BINARY_DIR}/messagecomposer
    ${CMAKE_SOURCE_DIR}/messagecomposer
    ${CMAKE_SOURCE_DIR}/templateparser
)
# Convenience macro to add messagecomposer unit tests.
macro( add_imageresize_test _source )
  set( _test ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(messagecomposer-autoresize-${_name})
  target_link_libraries( ${_name} KF5::Mime messagecomposer messageviewer messagecore templateparser KF5::PimTextEdit Qt5::Test    KF5::IdentityManagement KF5::Contacts 
  KF5::AkonadiCore
)
endmacro()

add_imageresize_test(imagescalingtest.cpp)
