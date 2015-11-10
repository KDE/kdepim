include_directories(${CMAKE_SOURCE_DIR}/libksieve/src
	${CMAKE_BUILD_DIR}/libksieve/src/ksiveui
        )

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

macro( add_vacation_test _source )
  set( _test ${_source}test.cpp
    ../${_source}.cpp
    ../legacy/vacationutils.cpp
    ../legacy/vacationscriptextractor.cpp
    ../vacationscriptextractor.cpp
    )
  configure_file(../../settings/sieve-editor.kcfg.cmake ${CMAKE_CURRENT_BINARY_DIR}/sieve-editor.kcfg)
  kconfig_add_kcfg_files(_test ../../settings/sieve-vacation.kcfgc ../../settings/sieve-editor.kcfgc)
  get_filename_component(_name ${_source} NAME_WE)
  ecm_qt_declare_logging_category(_test HEADER libksieve_debug.h IDENTIFIER LIBKSIEVE_LOG CATEGORY_NAME log_libksieve)
  add_executable(${_name} ${_test})
  add_test(vacation-${_name} ${_name})
  ecm_mark_as_test(vacation-${_name})
  add_definitions(-DVACATIONTESTDATADIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
  target_link_libraries(${_name}
    KF5::KSieveUi
    Qt5::Test
    KF5::IdentityManagement
    KF5::Mime
    )
endmacro()

add_vacation_test(vacationutils)
