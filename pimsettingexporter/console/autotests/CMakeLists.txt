set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
    ${CMAKE_BINARY_DIR}/pimsettingexporter/console/
    )

set( pimsettingexporterconsole_loginfiletest_SRCS loginfiletest.cpp ../loginfile.cpp )

add_executable( pimsettingexporterconsole_loginfiletest ${pimsettingexporterconsole_loginfiletest_SRCS})
add_test( pimsettingexporterconsole_loginfiletest pimsettingexporterconsole_loginfiletest)
ecm_mark_as_test(pimsettingexporterconsole_loginfiletest)
target_link_libraries( pimsettingexporterconsole_loginfiletest Qt5::Test)

