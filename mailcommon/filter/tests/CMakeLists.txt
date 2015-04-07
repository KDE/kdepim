
set(filterimporterexportergui_SRCS filterimporterexportergui.cpp)
add_executable(filterimporterexportergui ${filterimporterexportergui_SRCS})
ecm_mark_as_test(filterimporterexportergui)
target_link_libraries(filterimporterexportergui
  Qt5::Test
  Qt5::Core
  Qt5::Gui
  KF5::KDELibs4Support
  
  mailcommon
)

set(invalidfilterdialoggui_SRCS invalidfilterdialoggui.cpp)
add_executable(invalidfilterdialoggui ${invalidfilterdialoggui_SRCS})
ecm_mark_as_test(invalidfilterdialoggui)
target_link_libraries(invalidfilterdialoggui
  Qt5::Test
  Qt5::Core
  Qt5::Gui
  KF5::KDELibs4Support
  
  mailcommon
)

