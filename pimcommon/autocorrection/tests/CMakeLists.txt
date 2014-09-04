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
