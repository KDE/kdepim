include_directories(
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_BINARY_DIR}/messageviewer
)

# convenience macro to add qtest unit tests
macro(add_messageviewer_findbarunittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME messageviewer-${_name} ${_test} ../findbarbase.cpp)
  target_link_libraries( ${_name}
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDEUI_LIBRARY}
  )
endmacro ()

add_messageviewer_findbarunittest( findbarbasetest.cpp )
