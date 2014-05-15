include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(translator_gui_SRCS translator_gui.cpp)
add_executable(translator_gui TEST ${translator_gui_SRCS})
target_link_libraries(translator_gui
  pimcommon
)

# convenience macro to add qtest unit tests
macro(add_translator_unittest _source)
  set(_test ${_source} )
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME pimcommon-${_name} ${_test} )
  target_link_libraries( ${_name}
    ${QT_QTTEST_LIBRARY}
    pimcommon
  )
endmacro ()

add_translator_unittest( translatortest.cpp )


