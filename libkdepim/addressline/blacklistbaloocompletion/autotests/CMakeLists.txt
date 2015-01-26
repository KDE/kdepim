set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/libkdepim
)

set( libkdepim_blacklistbalooemailcompletiondialogtest_SRCS blacklistbalooemailcompletiondialogtest.cpp )
kde4_add_unit_test( libkdepim_blacklistbalooemailcompletiondialogtest ${libkdepim_blacklistbalooemailcompletiondialogtest_SRCS})
target_link_libraries( libkdepim_blacklistbalooemailcompletiondialogtest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdepim)


set( libkdepim_blacklistbalooemaillisttest_SRCS blacklistbalooemaillisttest.cpp )
kde4_add_unit_test( libkdepim_blacklistbalooemaillisttest ${libkdepim_blacklistbalooemaillisttest_SRCS})
target_link_libraries( libkdepim_blacklistbalooemaillisttest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} kdepim)

