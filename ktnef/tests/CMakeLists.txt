if(KDE4_BUILD_TESTS)

configure_file(config-ktnef-tests.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ktnef-tests.h)

macro(add_ktnef_test _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  kde4_automoc(${_test})
  kde4_add_executable(${_name} NOGUI RUN_UNINSTALLED ${_test})
  target_link_libraries(${_name} ktnef ${QT_QTTEST_LIBRARY})
  add_test(ktnef-${_name} ${EXECUTABLE_OUTPUT_PATH}/${_name})
endmacro(add_ktnef_test)

add_ktnef_test(parsertest.cpp)

endif(KDE4_BUILD_TESTS)
