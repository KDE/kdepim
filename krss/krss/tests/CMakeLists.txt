set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

macro(add_akonadi_isolated_test _testName)
  set(_targetName ${_testName})
  set(_srcList ${ARGN})
  kde4_add_executable(${_targetName} TEST ${_srcList})
  target_link_libraries(${_targetName} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} ${KDE4_KDECORE_LIBS} krss)

  # based on kde4_add_unit_test
  if (WIN32)
    get_target_property( _loc ${_targetName} LOCATION )
    set(_executable ${_loc}.bat)
  else (WIN32)
    set(_executable ${EXECUTABLE_OUTPUT_PATH}/${_targetName})
  endif (WIN32)
  if (UNIX)
    set(_executable ${_executable}.shell)
  endif (UNIX)

  add_test( krss-${_targetName} akonaditest -c ${CMAKE_CURRENT_SOURCE_DIR}/unittestenv/config.xml ${_executable} )
endmacro(add_akonadi_isolated_test)

macro(add_simple_test _testName)
  set(_targetName ${_testName})
  set(_srcList ${ARGN})
  kde4_add_unit_test(${_targetName} ${_srcList})
  target_link_libraries(${_targetName} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} ${KDE4_KDECORE_LIBS} krss)
endmacro(add_simple_test)

add_simple_test(feedcollectiontest feedcollectiontest.cpp)
add_simple_test(testxmlitemserializer testxmlitemserializer.cpp)
add_simple_test(testitem testitem.cpp)
add_simple_test(testcategory testcategory.cpp)
add_simple_test(testperson testperson.cpp)
add_simple_test(testenclosure testenclosure.cpp)
add_akonadi_isolated_test(tagprovidertest tagprovidertest.cpp)

kde4_add_executable(itemimporter itemimporter.cpp)
target_link_libraries(itemimporter ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} ${KDE4_KDECORE_LIBS} krss)
