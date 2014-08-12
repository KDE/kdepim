include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(yousendit_gui_SRCS yousendit_gui.cpp ../../tests/servicetestwidget.cpp)
add_executable(yousendit_gui ${yousendit_gui_SRCS})
target_link_libraries(yousendit_gui
  Qt5::Test
  KF5::KIOCore
  pimcommon
)

