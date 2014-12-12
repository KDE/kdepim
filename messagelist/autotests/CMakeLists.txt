include(ECMAddTests)

set(QT_REQUIRED_VERSION "5.2.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### next target ###############

# convenience macro to add qtest unit tests
macro(messagelist_unittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} ../messagelist_debug.cpp )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(messagelist-${_name})
  target_link_libraries( ${_name}
    messagecore
    messagelist
    Qt5::Test
  )
endmacro ()


messagelist_unittest(itemsizetest.cpp)
messagelist_unittest(quicksearchlinetest.cpp)
