if(NOT KDEPIM_NO_WEBKIT)
set(adblocksettings_gui_SRCS adblocksettings_gui.cpp)
add_executable(adblocksettings_gui ${adblocksettings_gui_SRCS})
target_link_libraries(adblocksettings_gui
  messageviewer
)

set(adblocksettingsdialog_gui_SRCS adblocksettingsdialog_gui.cpp)
add_executable(adblocksettingsdialog_gui ${adblocksettingsdialog_gui_SRCS})
target_link_libraries(adblocksettingsdialog_gui
  messageviewer
)

set(adblockblockableitemdialog_gui_SRCS adblockblockableitemdialog_gui.cpp)
add_executable(adblockblockableitemdialog_gui ${adblockblockableitemdialog_gui_SRCS})
target_link_libraries(adblockblockableitemdialog_gui
  KF5::KIOCore
  messageviewer
  kdepim
  Qt5::WebKitWidgets
)
endif()
