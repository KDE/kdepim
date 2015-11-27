
set(filterimporterexportergui_SRCS filterimporterexportergui.cpp)
add_executable(filterimporterexportergui ${filterimporterexportergui_SRCS})
ecm_mark_as_test(filterimporterexportergui)
target_link_libraries(filterimporterexportergui
  Qt5::Core
  Qt5::Gui
  KF5::MailCommon
)

set(invalidfilterdialoggui_SRCS invalidfilterdialoggui.cpp)
add_executable(invalidfilterdialoggui ${invalidfilterdialoggui_SRCS})
ecm_mark_as_test(invalidfilterdialoggui)
target_link_libraries(invalidfilterdialoggui
  Qt5::Core
  Qt5::Gui
  KF5::MailCommon
)

set(mndadvicedialoggui_SRCS mndadvicedialoggui.cpp)
add_executable(mndadvicedialoggui ${mndadvicedialoggui_SRCS})
ecm_mark_as_test(mndadvicedialoggui)
target_link_libraries(mndadvicedialoggui
  Qt5::Core
  Qt5::Gui
  KF5::MailCommon
)


set(kmfilterdialogtestgui_SRCS kmfilterdialogtestgui.cpp dummykernel.cpp)
add_executable(kmfilterdialogtestgui ${kmfilterdialogtestgui_SRCS})
ecm_mark_as_test(kmfilterdialogtestgui)
target_link_libraries(kmfilterdialogtestgui
  Qt5::Core
  Qt5::Gui
  KF5::MailCommon
)
