include_directories( ${CMAKE_SOURCE_DIR}/messagelist/core ${Boost_INCLUDE_DIR})
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

########### automated tests ###############

set(subjectutilstest_SRCS
    subjectutilstest.cpp
    ../core/subjectutils.cpp
)

kde4_add_unit_test(subjectutilstest TESTNAME messagelistcore-subjectutilstest NOGUI ${subjectutilstest_SRCS})
target_link_libraries(subjectutilstest ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} messagelist)
