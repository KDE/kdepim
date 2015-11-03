include_directories(${libksieve_SOURCE_DIR}
        ${libksieve_SOURCE_DIR}/src/ksieveui/scriptsparsing
        )


set(autocreatescripts_gui_SRCS autocreatescripts_gui.cpp)
add_executable(autocreatescripts_gui ${autocreatescripts_gui_SRCS})
target_link_libraries(autocreatescripts_gui
  KF5::KSieveUi
  KF5::PimCommon
)

set(parsingscript_gui_SRCS parsingscript_gui.cpp)
add_executable(parsingscript_gui ${parsingscript_gui_SRCS})
target_link_libraries(parsingscript_gui
  KF5::KIOCore
  KF5::KSieveUi
  KF5::KSieve
  KF5::PimCommon
)


