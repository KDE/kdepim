include_directories(${CMAKE_SOURCE_DIR}/kaddressbook/
)


set(gravatarcreatedialog_gui_SRCS gravatarcreatedialog_gui.cpp ../widgets/gravatarcreatedialog.cpp ../widgets/gravatarcreatewidget.cpp ../job/gravatarcreatejob.cpp)
kde4_add_executable(gravatarcreatedialog_gui TEST ${gravatarcreatedialog_gui_SRCS})
target_link_libraries(gravatarcreatedialog_gui
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  pimcommon
)

