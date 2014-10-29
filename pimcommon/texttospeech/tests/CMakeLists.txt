include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(texttospeechwidget_gui_SRCS texttospeechwidget_gui.cpp)
add_executable(texttospeechwidget_gui ${texttospeechwidget_gui_SRCS})
target_link_libraries(texttospeechwidget_gui
  Qt5::Test
  pimcommon
)

set(texttospeechgui_SRCS texttospeechgui.cpp)
add_executable(texttospeechgui ${texttospeechgui_SRCS})
target_link_libraries(texttospeechgui
  Qt5::Test
  pimcommon
)

set(texttospeechconfiggui_SRCS texttospeechconfiggui.cpp)
add_executable(texttospeechconfiggui ${texttospeechconfiggui_SRCS})
target_link_libraries(texttospeechconfiggui
  Qt5::Test
  pimcommon
)

