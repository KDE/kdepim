include_directories(${CMAKE_SOURCE_DIR}/libksieve
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui
        ${CMAKE_BINARY_DIR}/libksieve
        ${CMAKE_BINARY_DIR}/libksieve/ksieveui
        )


set(vacation_multi_server_SRCS
  main.cpp
)

KDE4_ADD_EXECUTABLE(vacationmultiservertest ${vacation_multi_server_SRCS} )
TARGET_LINK_LIBRARIES(vacationmultiservertest ${KDE4_KDEUI_LIBS} ksieveui ksieve)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

macro( add_vacation_test _source )
  set( _test ${_source}test.cpp
    ../${_source}.cpp
    ../vacationscriptextractor.cpp
    ${CMAKE_BINARY_DIR}/libksieve/ksieveui/sieve-vacation.cpp
    ${CMAKE_SOURCE_DIR}/libksieve/parser/lexer.cpp
    ${CMAKE_SOURCE_DIR}/libksieve/parser/parser.cpp
    ${CMAKE_SOURCE_DIR}/libksieve/parser/utf8validator.cpp
    )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME vacation-${_name} ${_test} )
  set_target_properties(${_name} PROPERTIES COMPILE_FLAGS -DVACATIONTESTDATADIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")
  target_link_libraries( ${_name}
  ksieveui
  kmanagesieve
  ksieve
  ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KMIME_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS})
endmacro()

add_vacation_test( vacationutils )