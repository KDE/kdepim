
project(bnf_selection)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../
)

set( bnf_selection_srcs
  main.cpp
  mainwindow.cpp
)

kde4_add_executable(bnf_selection
  ${bnf_selection_srcs}
)

target_link_libraries(bnf_selection
  kdepimmobileui
  ${QT_QTGUI_LIBRARIES}
  ${QT_QTCORE_LIBRARIES}
)

