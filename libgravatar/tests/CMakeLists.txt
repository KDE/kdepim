set(gravatarwidget_gui_SRCS gravatarwidget_gui.cpp)
add_executable(gravatarwidget_gui ${gravatarwidget_gui_SRCS})
ecm_mark_as_test(gravatarwidget_gui)
target_link_libraries(gravatarwidget_gui
  Qt5::Core
  Qt5::Gui
  KF5::Gravatar
  KF5::I18n
  Qt5::Widgets
  KF5::CoreAddons
)

