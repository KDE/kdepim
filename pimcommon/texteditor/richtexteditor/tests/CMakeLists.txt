include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


set(richtexteditor_gui_SRCS richtexteditor_gui.cpp)
add_executable(richtexteditor_gui TEST ${richtexteditor_gui_SRCS})
target_link_libraries(richtexteditor_gui
  
  
  
  
  pimcommon
)

