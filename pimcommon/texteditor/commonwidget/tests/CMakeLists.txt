include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon/
)
# convenience macro to add qtest unit tests
macro(add_plaintext_unittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME pimcommon-${_name} ${_test})
  target_link_libraries( ${_name}
    pimcommon
    ${QT_QTTEST_LIBRARY}
    
    
  )
endmacro ()

add_plaintext_unittest( textgotolinewidgettest.cpp )

