PROJECT( composerhtmleditor )

include_directories(
  ${CMAKE_SOURCE_DIR}/composereditor-ng )

set(composerhtmlSources
  main.cpp
  composerhtmleditor.cpp
)

add_executable(composerhtmleditor ${composerhtmlSources} )
target_link_libraries(composerhtmleditor Qt5::WebKitWidgets  KF5::KIOCore composereditorng )

install(TARGETS composerhtmleditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  composerhtmleditorui.rc
  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/composerhtmleditor)

