set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon
  ${CMAKE_BINARY_DIR}/pimcommon
)

set(autocorrection_gui_SRCS autocorrection_gui.cpp)
add_executable(autocorrection_gui ${autocorrection_gui_SRCS})
target_link_libraries(autocorrection_gui
  Qt5::Test
  KF5::KIOCore
  pimcommon
)


set(richtexteditwithautocorrection_gui_SRCS richtexteditwithautocorrection_gui.cpp)
add_executable(richtexteditwithautocorrection_gui ${richtexteditwithautocorrection_gui_SRCS})
target_link_libraries(richtexteditwithautocorrection_gui
  Qt5::Test
  KF5::KIOCore
  pimcommon
)


set( lineeditwithautocorrectiontest_SRCS lineeditwithautocorrectiontest.cpp )
add_executable( lineeditwithautocorrectiontest ${lineeditwithautocorrectiontest_SRCS})
add_test(lineeditwithautocorrectiontest lineeditwithautocorrectiontest)
ecm_mark_as_test(lineeditwithautocorrectiontest)
target_link_libraries( lineeditwithautocorrectiontest Qt5::Test  pimcommon)

set( richtexteditwithautocorrectiontest_SRCS richtexteditwithautocorrectiontest.cpp )
add_executable( richtexteditwithautocorrectiontest ${richtexteditwithautocorrectiontest_SRCS})
add_test(richtexteditwithautocorrectiontest richtexteditwithautocorrectiontest)
ecm_mark_as_test(richtexteditwithautocorrectiontest)
target_link_libraries( richtexteditwithautocorrectiontest Qt5::Test  pimcommon)


