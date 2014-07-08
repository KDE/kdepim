PROJECT( composerhtmleditor )

include_directories(
  ${CMAKE_SOURCE_DIR}/composereditor-ng )

set(composerhtmlSources
  main.cpp
  composerhtmleditor.cpp
)

add_executable(composerhtmleditor ${composerhtmlSources} )
TARGET_LINK_LIBRARIES(composerhtmleditor Qt5::WebKitWidgets  KF5::KIOCore composereditorng )

install(TARGETS composerhtmleditor ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  composerhtmleditorui.rc
  DESTINATION  ${KXMLGUI_INSTALL_DIR}/composerhtmleditor)

