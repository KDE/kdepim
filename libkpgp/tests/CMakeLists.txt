set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

kde4_add_executable(testkeyselectiondialog TEST testkeyselectiondialog.cpp)
target_link_libraries(testkeyselectiondialog kpgp)
