include_directories(${CMAKE_SOURCE_DIR}/calendarviews
)


set(createcolorgui_test createcolorgui_test.cpp)
kde4_add_executable(createcolorgui_test TEST ${createcolorgui_test})
target_link_libraries(createcolorgui_test
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  eventviews
)

