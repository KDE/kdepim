include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(customtextedit_gui_SRCS customtextedit_gui.cpp)
add_executable(customtextedit_gui ${customtextedit_gui_SRCS})
target_link_libraries(customtextedit_gui
  Qt5::Test
  pimcommon
)

set(customtoolswidget_gui_SRCS customtoolswidget_gui.cpp)
add_executable(customtoolswidget_gui ${customtoolswidget_gui_SRCS})
target_link_libraries(customtoolswidget_gui
  Qt5::Test
  pimcommon
)

set(slidecontainer_gui_SRCS slidecontainer_gui.cpp)
add_executable(slidecontainer_gui ${slidecontainer_gui_SRCS})
ecm_mark_as_test(slidecontainer_gui)
target_link_libraries(slidecontainer_gui
  Qt5::Test
  Qt5::Core
  Qt5::Gui
  KF5::KDELibs4Support
  
  pimcommon
)

set(splittercollapser_gui_SRCS splittercollapsergui_test.cpp)
add_executable(splittercollapsergui_test ${splittercollapser_gui_SRCS})
ecm_mark_as_test(splittercollapsergui_test)
target_link_libraries(splittercollapsergui_test
  Qt5::Test
  Qt5::Core
  Qt5::Gui
  KF5::KDELibs4Support
  
  pimcommon
)

