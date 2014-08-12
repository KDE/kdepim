set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_executable(testkeyselectiondialog testkeyselectiondialog.cpp)
target_link_libraries(testkeyselectiondialog kpgp    )
