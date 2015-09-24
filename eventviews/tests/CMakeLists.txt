set(createcolorgui_test createcolorgui_test.cpp)
add_executable(createcolorgui_test ${createcolorgui_test})
ecm_mark_as_test(createcolorgui_test)
target_link_libraries(createcolorgui_test
  Qt5::Core
  Qt5::Gui
  KF5::EventViews
)

