include_directories(${CMAKE_SOURCE_DIR}/libksieve
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui/scriptsparsing
        )


set(autocreatescripts_gui_SRCS autocreatescripts_gui.cpp)
add_executable(autocreatescripts_gui ${autocreatescripts_gui_SRCS})
target_link_libraries(autocreatescripts_gui
  ksieveui
  pimcommon
)

set(parsingscript_gui_SRCS parsingscript_gui.cpp)
add_executable(parsingscript_gui ${parsingscript_gui_SRCS})
target_link_libraries(parsingscript_gui
  KF5::KIOCore
  ksieveui
  ksieve
  pimcommon
)


