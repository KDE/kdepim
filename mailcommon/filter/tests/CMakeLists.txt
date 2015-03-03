
set(filterimporterexportergui_SRCS filterimporterexportergui.cpp)
kde4_add_executable(filterimporterexportergui TEST ${filterimporterexportergui_SRCS})
target_link_libraries(filterimporterexportergui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  mailcommon
)

