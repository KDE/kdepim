set(richtextcomposerwidget_gui_SRCS richtextcomposerwidget_gui.cpp)
add_executable(richtextcomposerwidget_gui ${richtextcomposerwidget_gui_SRCS})
target_link_libraries(richtextcomposerwidget_gui
  KF5::PimCommon KF5::MessageComposer KF5::XmlGui
)

