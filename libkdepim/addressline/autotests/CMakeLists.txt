set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
)

set( libkdepim_baloocompletionemailtest_SRCS baloocompletionemailtest.cpp ../baloocompletionemail.cpp)
kde4_add_unit_test( libkdepim_baloocompletionemailtest ${libkdepim_baloocompletionemailtest_SRCS})
target_link_libraries( libkdepim_baloocompletionemailtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS})

