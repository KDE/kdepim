set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
)

set( libkdepim_baloocompletionemailtest_SRCS baloocompletionemailtest.cpp ../baloocompletionemail.cpp)
add_executable( libkdepim_baloocompletionemailtest ${libkdepim_baloocompletionemailtest_SRCS})
add_test(libkdepim_baloocompletionemailtest libkdepim_baloocompletionemailtest)
ecm_mark_as_test(libkdepim_baloocompletionemailtest)
target_link_libraries( libkdepim_baloocompletionemailtest Qt5::Test )

