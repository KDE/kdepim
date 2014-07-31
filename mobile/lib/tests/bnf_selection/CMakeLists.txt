
project(bnf_selection)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../
)

set( bnf_selection_srcs
  main.cpp
  mainwindow.cpp
)

add_executable( bnf_selection ${bnf_selection_srcs} )
add_test( bnf_selection bnf_selection )
ecm_mark_as_test(bnf_selection)

target_link_libraries(bnf_selection
  kdepimmobileui
  ${QT_QTGUI_LIBRARIES}
  ${QT_QTCORE_LIBRARIES}
)

