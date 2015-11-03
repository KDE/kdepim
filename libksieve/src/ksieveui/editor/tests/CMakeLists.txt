include_directories(${libksieve_SOURCE_DIR}
        ${libksieve_SOURCE_DIR}/src/ksieveui/scriptsparsing
        )


set(editorsieve_gui_SRCS editorsieve_gui.cpp)
add_executable(editorsieve_gui ${editorsieve_gui_SRCS})
target_link_libraries(editorsieve_gui
  KF5::KSieveUi
  KF5::PimCommon
)

