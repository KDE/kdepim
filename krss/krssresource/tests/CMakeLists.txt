set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

# path to test data
add_definitions(-DTESTDATAPATH=\\"${CMAKE_CURRENT_SOURCE_DIR}\\")

macro(add_akonadi_isolated_test _testName _configName)
  set(_targetName ${_testName})
  set(_configFile ${_configName})
  set(_srcList ${ARGN})

  # add the dbus interace to every test (easier than adding to particular tests only)
  qt4_add_dbus_interface(_srcList ../org.kde.krss.xml krssinterface)

  kde4_add_executable(${_targetName} TEST ${_srcList})
  target_link_libraries(${_targetName} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} ${KDE4_KDECORE_LIBS} krss krssresource)

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

  find_program(_testrunner akonaditest)

  add_test( krss-${_targetName} ${_testrunner} -c ${CMAKE_CURRENT_SOURCE_DIR}/unittestenv/${_configFile} ${_executable} )
endmacro(add_akonadi_isolated_test)

macro(add_simple_test _testName)
  set(_targetName ${_testName})
  set(_srcList ${ARGN})
  kde4_add_unit_test(${_targetName} ${_srcList})
  target_link_libraries(${_targetName} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} ${KDE4_KDECORE_LIBS} krss krssresource)
endmacro(add_simple_test)

add_simple_test(rssitemsynctest rssitemsynctest.cpp)
add_simple_test(itemimportreadertest itemimportreadertest.cpp)
add_akonadi_isolated_test(importopmljobtest config1.xml importopmljobtest.cpp)
add_akonadi_isolated_test(importitemsjobtest config2.xml importitemsjobtest.cpp)
