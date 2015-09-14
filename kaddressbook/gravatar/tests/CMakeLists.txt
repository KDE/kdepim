include_directories(${CMAKE_SOURCE_DIR}/kaddressbook/ ${CMAKE_BINARY_DIR}/kaddressbook/
)


set(gravatarupdatedialog_gui_SRCS gravatarupdatedialog_gui.cpp ../widgets/gravatarupdatedialog.cpp ../widgets/gravatarupdatewidget.cpp ../job/gravatarupdatejob.cpp ${CMAKE_BINARY_DIR}/kaddressbook/kaddressbook_debug.cpp)
add_executable(gravatarupdatedialog_gui ${gravatarupdatedialog_gui_SRCS})
ecm_mark_as_test(gravatarupdatedialog_gui)
target_link_libraries(gravatarupdatedialog_gui
  Qt5::Gui
  KF5::PimCommon
)

