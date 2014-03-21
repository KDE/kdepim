include_directories(${CMAKE_SOURCE_DIR}/libksieve
        ${CMAKE_SOURCE_DIR}/libksieve/ksieveui/scriptsparsing
        )


set(autocreatescripts_gui_SRCS autocreatescripts_gui.cpp)
kde4_add_executable(autocreatescripts_gui TEST ${autocreatescripts_gui_SRCS})
target_link_libraries(autocreatescripts_gui
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ksieveui
  pimcommon
)

set(parsingscript_gui_SRCS parsingscript_gui.cpp)
kde4_add_executable(parsingscript_gui TEST ${parsingscript_gui_SRCS})
target_link_libraries(parsingscript_gui
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


