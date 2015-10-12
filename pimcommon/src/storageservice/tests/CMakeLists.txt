set(storageservice_gui_SRCS storageservice_gui.cpp testsettingsjob.cpp )
add_executable(storageservice_gui ${storageservice_gui_SRCS})
target_link_libraries(storageservice_gui
  KF5::PimCommon
  KF5::XmlGui
)

