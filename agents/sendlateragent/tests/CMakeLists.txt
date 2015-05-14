set(sendlaterinfo_gui_SRCS sendlaterinfo_gui.cpp )
add_executable(sendlaterinfo_gui ${sendlaterinfo_gui_SRCS})
target_link_libraries(sendlaterinfo_gui
  sendlater
  KF5::I18n
)
