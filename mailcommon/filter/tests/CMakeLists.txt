
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
kde4_add_executable(invalidfilterdialoggui TEST ${invalidfilterdialoggui_SRCS})
target_link_libraries(invalidfilterdialoggui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  mailcommon
)

