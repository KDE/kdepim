set(translator_gui_SRCS translator_gui.cpp)
add_executable(translator_gui ${translator_gui_SRCS})
target_link_libraries(translator_gui
  KF5::PimCommon KF5::PimTextEdit
)

