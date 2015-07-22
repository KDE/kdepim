add_executable(sievescriptdebugger_gui sievescriptdebugger_gui.cpp)

target_link_libraries(sievescriptdebugger_gui
  ksieve
  ksieveui
)
