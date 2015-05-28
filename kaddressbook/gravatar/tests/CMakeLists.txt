include_directories(${CMAKE_SOURCE_DIR}/kaddressbook/
)


set(gravatarcreatedialog_gui_SRCS gravatarcreatedialog_gui.cpp ../widgets/gravatarcreatedialog.cpp ../widgets/gravatarcreatewidget.cpp ../job/gravatarcreatejob.cpp)
add_executable(gravatarcreatedialog_gui ${gravatarcreatedialog_gui_SRCS})
ecm_mark_as_test(gravatarcreatedialog_gui)
target_link_libraries(gravatarcreatedialog_gui
  Qt5::Gui
  KF5::KDELibs4Support
  
  pimcommon
)

