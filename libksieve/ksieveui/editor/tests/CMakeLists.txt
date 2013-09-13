include_directories(${CMAKE_SOURCE_DIR}/libksieve
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui/scriptsparsing
        )


set(editorsieve_gui_SRCS editorsieve_gui.cpp)
kde4_add_executable(editorsieve_gui TEST ${editorsieve_gui_SRCS})
target_link_libraries(editorsieve_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ksieveui
  pimcommon
)

set(editor_gui_SRCS editorsieve_gui.cpp)
kde4_add_executable(editorsieve_gui TEST ${editorsieve_gui_SRCS})
target_link_libraries(editorsieve_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ksieveui
  ksieve
  pimcommon
)


