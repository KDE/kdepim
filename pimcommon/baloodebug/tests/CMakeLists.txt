
set(baloodebug_dialog_gui_SRCS baloodebug_dialog_gui.cpp)
add_executable(baloodebug_dialog_gui ${baloodebug_dialog_gui_SRCS})
ecm_mark_as_test(baloodebug_dialog_gui)
target_link_libraries(baloodebug_dialog_gui
  Qt5::Core
  Qt5::Gui
  KF5::KDELibs4Support
  pimcommon
)

