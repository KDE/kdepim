set(autocorrection_gui_SRCS autocorrection_gui.cpp)
add_executable(autocorrection_gui ${autocorrection_gui_SRCS})
target_link_libraries(autocorrection_gui
  KF5::KIOCore
  KF5::PimCommon
  KF5::PimTextEdit
)


set(richtexteditwithautocorrection_gui_SRCS richtexteditwithautocorrection_gui.cpp)
add_executable(richtexteditwithautocorrection_gui ${richtexteditwithautocorrection_gui_SRCS})
target_link_libraries(richtexteditwithautocorrection_gui
  KF5::KIOCore
  KF5::PimCommon
  KF5::PimTextEdit
)

